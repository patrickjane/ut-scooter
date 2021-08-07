// **************************************************************************
// class Provider
// 02.07.2021
// Base class for scooter provider API implementations
// **************************************************************************
// MIT License
// Copyright © 2021 Patrick Fial
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// **************************************************************************
// includes
// **************************************************************************

#ifndef PROVIDER_H
#define PROVIDER_H

#include <QObject>
#include <QVariantMap>

#include "types.hpp"
#include "logger.h"

// **************************************************************************
// forwards
// **************************************************************************

class QGeoPositionInfoSource;

namespace network {
   class Network;
}

// **************************************************************************
// namespace scooter
// **************************************************************************

namespace scooter
{
   // **************************************************************************
   // class Provider
   // **************************************************************************

   class Provider : public Logger
   {
         Q_OBJECT

      public:
         Provider() = delete;
         Provider(QObject* parent = nullptr) = delete;
         explicit Provider(network::Network* net, QGeoPositionInfoSource* position, QObject* parent = nullptr);
         virtual ~Provider() {}

         virtual void init() = 0;

         virtual bool isLoggedIn() = 0;
         virtual bool isReady() { return fReady; }

         virtual void login(QString loginId, QString password = "") = 0;
         virtual void logout();
         virtual void confirmLogin(QString) {}
         virtual void refreshLogin() = 0;
         virtual void getProfile() = 0;
         virtual void reloadConfig(QGeoCoordinate coordinates) = 0;

         virtual void reloadScooters(QGeoCoordinate coordinates, int radius, ResultCallback<ScooterMap>) = 0;
         virtual void reloadAreas(QGeoCoordinate coordinates, int radius) = 0;

         virtual void ringScooter(QString id, QGeoCoordinate coordinate) = 0;
         virtual void scanScooter(QString qrCode, QGeoCoordinate coordinates, ResultCallback<Scooter>) = 0;
         virtual void startRide(QString unlockId, QGeoCoordinate coordinate, ResultCallback<QString>) = 0;
         virtual void stopRide(QString rideId, QGeoCoordinate coordinate, SupplementaryCallback<QString>) = 0;

         QString getAccountId() { return account; }

      signals:
         void ready();
         void activeRideLoaded(QString rideId, QDateTime started);
         void notify(QString title, QString message);
         void error(QString error);
         void scootersChanged(ScooterMap scooters);
         void areasChanged(const AreaList& areas);
         void confirmLoginNeeded();
         void loginStatusChanged(bool loggedIn, QString account);
         void profile(QVariantMap profile);

      protected:
         void reportNetworkError(QString prefix, int err, int code, bool bodyEmpty);

         bool fReady;
         network::Network* net;
         QString account;
         PriceInfo priceInfo;
         QGeoPositionInfoSource* position;
         AreaList areas;
   };
}

#endif // PROVIDER_H
