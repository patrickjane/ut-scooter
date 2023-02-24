// **************************************************************************
// class Scooters
// Controller class/bridge between providers & QML
// **************************************************************************
// MIT License
// Copyright © 2023 Patrick Fial
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the “Software”), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright notice and this
// permission notice shall be included in all copies or substantial portions of the Software. THE
// SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// **************************************************************************
// includes
// **************************************************************************

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGeoPositionInfoSource>
#include <QIODevice>
#include <QSettings>
#include <QStandardPaths>
#include <QVariant>
#include <QVariantMap>

#include "async.hpp"
#include "providerbird.h"
#include "providernextbike.h"
#include "providerstub.h"
#include "scooters.h"

namespace C {
#include <libintl.h>
}

// **************************************************************************
// namespace scooter
// **************************************************************************

namespace scooter {
// **************************************************************************
// ctor/dtor
// **************************************************************************

Scooters::Scooters(QObject* parent) : Logger(parent), net(this), activeRide(nullptr)
{
    // GPS

    position = QGeoPositionInfoSource::createDefaultSource(0);

    if (!position->lastKnownPosition().isValid())
        position->requestUpdate(0);

    // restore settings from previous app run

    QSettings settings;

    if (settings.contains("app/activeRide")) {
        auto ride = settings.value("app/activeRide").toMap();
        activeRide = new ActiveRide {ride["rideId"].toString(), ride["provider"].toString(),
                                     ride["started"].toDateTime(), ride["scooterId"].toString()};
    }

    connect(&net, &network::Network::networkError, this,
            [this](int code, QString text) { emit networkError("network", code, text); });

    // providers

    providers[kProviderBird] = new ProviderBird(&net, position, this);
    providers[kProviderNextbike] = new ProviderNextbike(&net, position, this);

    // connect/forward/gather signals

    for (auto provider : providers.toStdMap()) {
        auto providerName = provider.first;

        providerNames << providerName;

        connect(provider.second, &Provider::confirmLoginNeeded, this,
                [this, providerName]() { emit confirmLoginNeeded(providerName); });

        connect(provider.second, &Provider::loginStatusChanged, this,
                [this, providerName](bool loggedIn, QString account) {
                    emit loginStatusChanged(providerName, loggedIn, account);
                });

        connect(provider.second, &Provider::error, this,
                [this, providerName](QString text) { emit error(providerName, text); });

        connect(provider.second, &Provider::notify, this,
                [this, providerName](QString title, QString message) {
                    emit notify(providerName, title, message);
                });

        connect(provider.second, &Provider::profile, this,
                [this, providerName](QVariantList p) { emit profile(providerName, p); });

        connect(provider.second, &Provider::logMessage, this,
                [this, providerName](Severity severity, QString message) {
                    log(severity, providerName, message);
                });

        connect(provider.second, &Provider::ready, this, [this]() {
            foreach (auto provider, providers) {
                if (!provider->isReady())
                    return;
            }

            emit ready();
        });

        connect(provider.second, &Provider::scootersChanged, this,
                [this, providerName](const ScooterList& scooters) {
                    QVariantList res;
                    res << scooters;

                    emit scootersChanged(providerName, res);
                });

        connect(provider.second, &Provider::areasChanged, this,
                [this, providerName](const AreaList& areas) {
                    QVariantList res;
                    res << areas;

                    emit areasChanged(providerName, res);
                });

        connect(provider.second, &Provider::terminalsChanged, this,
                [this, providerName](const TerminalList& terminals) {
                    QVariantList res;
                    res << terminals;

                    emit terminalsChanged(providerName, res);
                });

        connect(provider.second, &Provider::activeRideLoaded, this,
                [this, providerName](QString rideId, QDateTime started, QString scooterId) {
                    activeRide = new ActiveRide {rideId, providerName, started, scooterId};
                    emit activeRideChanged(QVariant(*activeRide), "", "");
                });
    }
}

Scooters::~Scooters()
{
    for (auto it = providers.begin(); it != providers.end(); it++)
        delete it.value();

    providers.clear();
}

// **************************************************************************
// init
// **************************************************************************

void Scooters::init()
{
    for (auto provider : providers.toStdMap()) {
        qDebug() << "Init provider: " << provider.first;
        provider.second->init();
    }
}

// **************************************************************************
// login
// **************************************************************************

void Scooters::login(QString providerName, QString loginId, QString password)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->login(loginId, password);
}

// **************************************************************************
// confirmLogin
// **************************************************************************

void Scooters::confirmLogin(QString providerName, QString confirmation)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->confirmLogin(confirmation);
}

// **************************************************************************
// logout
// **************************************************************************

void Scooters::logout(QString providerName)
{
    if (activeRide && activeRide->provider == providerName) {
        emit error(providerName, C::gettext("Can not log out while ride is active"));
        return;
    }

    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->logout();
}

// **************************************************************************
// getProfile
// **************************************************************************

void Scooters::getProfile(QString providerName)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->getProfile();
}

// **************************************************************************
// getAccountHistory
// **************************************************************************

void Scooters::getAccountHistory(QString providerName)
{
    providers[providerName]->getAccountHistory(
      [this](QString err, QString contents) { emit accountHistoryLoaded(err, contents); });
}

// **************************************************************************
// setCountry
// **************************************************************************

void Scooters::setCountry(QString providerName, QString countryName)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->setCountry(countryName);
}

// **************************************************************************
// setCity
// **************************************************************************

void Scooters::setCity(QString providerName, QString cityName, QString domain)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->setCity(cityName, domain);
}

// **************************************************************************
// getAccountId
// **************************************************************************

QString Scooters::getAccountId(QString providerName)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return "";
    }

    return providers[providerName]->getAccountId();
}

// **************************************************************************
// getCountry
// **************************************************************************

QString Scooters::getCountry(QString providerName)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return "";
    }

    return providers[providerName]->getCountry();
}

// **************************************************************************
// getCity
// **************************************************************************

QString Scooters::getCity(QString providerName)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return "";
    }

    return providers[providerName]->getCity();
}

// **************************************************************************
// isLoggedIn
// **************************************************************************

Q_INVOKABLE bool Scooters::isLoggedIn(QString providerName)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return false;
    }

    return providers[providerName]->isLoggedIn();
}

// **************************************************************************
// reloadScooters
// **************************************************************************

void Scooters::reloadScooters(QGeoCoordinate coordinates, int radius)
{
    for (auto provider : providers.toStdMap()) {
        provider.second->reloadScooters(coordinates, radius);
    }
}

// **************************************************************************
// reloadAreas
// **************************************************************************

void Scooters::reloadAreas(QGeoCoordinate coordinates, int radius)
{
    for (auto provider : providers.toStdMap()) {
        provider.second->reloadAreas(coordinates, radius);
    }
}

// **************************************************************************
// reloadTerminals
// **************************************************************************

void Scooters::reloadTerminals(QGeoCoordinate coordinates, int radius)
{
    for (auto provider : providers.toStdMap()) {
        provider.second->reloadTerminals(coordinates, radius);
    }
}

// **************************************************************************
// ringScooter
// **************************************************************************

Q_INVOKABLE void Scooters::ringScooter(QString providerName, QString id, QGeoCoordinate coordinate)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->ringScooter(id, coordinate);
}

// **************************************************************************
// scanScooter
// **************************************************************************

Q_INVOKABLE void Scooters::scanScooter(QString providerName, QString qrCode,
                                       QGeoCoordinate coordinates)
{
    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->scanScooter(qrCode, coordinates, [this](QString err, Scooter scooter) {
        if (!err.isEmpty()) {
            emit error(C::gettext("Ride"), err);
            return;
        }

        emit scooterScanned(QVariant(scooter));
    });
}

// **************************************************************************
// startRide
// **************************************************************************

Q_INVOKABLE void Scooters::startRide(QString providerName, QString unlockId,
                                     QGeoCoordinate coordinate, bool reUnlock)
{
    if (activeRide && !reUnlock) {
        emit error(C::gettext("Ride"), C::gettext("Ride already active, can not start new ride"));
        return;
    }

    if (!providers.contains(providerName)) {
        emit error(providerName, C::gettext("Provider unknown/not supported"));
        return;
    }

    providers[providerName]->startRide(
      unlockId, coordinate,
      [this, providerName, reUnlock](QString err, QString rideId, QString scooterId) {
          if (!err.isEmpty()) {
              emit error(C::gettext("Ride"), err);
              return;
          }

          if (!reUnlock) {
              activeRide = new ActiveRide {rideId, providerName, QDateTime::currentDateTime(),
                                           scooterId};
              emit activeRideChanged(QVariant(*activeRide), "", "");
          } else {
              emit notify(providerName, C::gettext("Ride"),
                          C::gettext("Successfully unlocked again"));
          }
      });
}

// **************************************************************************
// stopRide
// **************************************************************************

Q_INVOKABLE void Scooters::stopRide(QGeoCoordinate coordinate)
{
    if (!activeRide) {
        emit error(C::gettext("Ride"), C::gettext("No ride active, can not stop ride"));
        return;
    }

    providers[activeRide->provider]->stopRide(
      activeRide->rideId, coordinate, [this](QString err, QString info, QString photoErr) {
          emit activeRideChanged(QVariant(), err.isEmpty() ? photoErr : err, info);

          if (!info.isEmpty()) {
              delete activeRide;
              activeRide = nullptr;
          }
      });
}

// **************************************************************************
// checkRide
// **************************************************************************

Q_INVOKABLE void Scooters::checkActiveRide(bool interval)
{
    if (!activeRide) {
        emit error(C::gettext("Ride"), C::gettext("No ride active, can not check ride"));
        return;
    }

    providers[activeRide->provider]->checkActiveRide(
      activeRide->rideId, [this, interval](QString err, QString info, QString photoErr) {
          emit rideChecked(interval ? "" : err, info);

          if (!info.isEmpty()) {
              delete activeRide;
              activeRide = nullptr;
          }
      });
}

// **************************************************************************
// exportLogs
// **************************************************************************

Q_INVOKABLE QString Scooters::exportLogs(QString logs)
{
    auto dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto filePath = dataPath + "/logs.txt";

    QDir dir(dataPath);

    if (!dir.exists()) {
        bool res = dir.mkpath(dataPath);

        if (!res) {
            QString(C::gettext("App data storage location inaccessible")) + " ("
              + QString(C::gettext("can't create folder: ")) + " " + dir.absolutePath() + ")";
            return "";
        }
    }

    QFile file(filePath);

    auto ok = file.open(QIODevice::WriteOnly | QIODevice::Truncate);

    if (ok)
        ok = file.write(logs.toUtf8());

    if (!ok) {
        emit error(C::gettext("Log export"),
                   C::gettext("Failed to create log export: ") + file.errorString());
        return "";
    }

    file.close();

    return filePath;
}

} // namespace scooter
