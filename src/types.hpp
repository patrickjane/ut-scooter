// **************************************************************************
// class Scooter
// Utility classes
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

#ifndef TYPES_HPP
#define TYPES_HPP

#include <QDateTime>
#include <QDebug>
#include <QGeoCoordinate>
#include <QString>
#include <functional>
#include <map>

// **************************************************************************
// namespace scooter
// **************************************************************************

namespace scooter {
const QString kProviderBird = "bird";
const QString kProviderVoi = "voi";
const QString kProviderLime = "lime";
const QString kProviderBolt = "bolt";
const QString kProviderTier = "tier";
const QString kProviderWind = "wind";
const QString kProviderNextbike = "nextbike";

template <typename T>
using ResultCallback = std::function<void(QString, T)>;

template <typename T>
using SupplementaryCallback = std::function<void(QString, T, QString)>;

struct Scooter {
    QString mapId;
    QString unlockId;
    QString qrCode;
    QString provider;
    QGeoCoordinate coordinate;
    int battery;
    double range;
    bool captive;
    bool available;
    double basePrice;
    double pricePerMin;
    QString priceString;
    QString description;

    explicit operator QVariant() const
    {
        QVariantMap m;
        m.insert("mapId", mapId);
        m.insert("unlockId", unlockId);
        m.insert("provider", provider);
        m.insert("coordinate", QVariant::fromValue(coordinate));
        m.insert("battery", QVariant(battery));
        m.insert("range", QVariant(range));
        m.insert("captive", QVariant(captive));
        m.insert("available", QVariant(available));
        m.insert("basePrice", QVariant(basePrice));
        m.insert("pricePerMin", QVariant(pricePerMin));
        m.insert("priceString", priceString);
        m.insert("description", description);

        return m;
    }
};

using ScooterList = QList<Scooter>;

inline QVariantList& operator<<(QVariantList& list, const ScooterList& scooters)
{
    for (const Scooter& a : scooters)
        list << static_cast<QVariant>(a);

    return list;
}

struct ActiveRide {
    QString rideId;
    QString provider;
    QDateTime started;
    QString scooterId;

    explicit operator QVariant() const
    {
        QVariantMap m;
        m.insert("rideId", rideId);
        m.insert("provider", provider);
        m.insert("started", QVariant(started));
        m.insert("scooterId", scooterId);
        return m;
    }
};

struct PriceInfo {
    double basePrice;
    double pricePerMin;
    QString priceString;

    void clear()
    {
        basePrice = pricePerMin = 0.0;
        priceString.clear();
    }

    explicit operator QVariant() const
    {
        QVariantMap m;
        m.insert("basePrice", basePrice);
        m.insert("pricePerMin", pricePerMin);
        m.insert("priceString", priceString);
        return m;
    }
};

enum AreaType {
    atNoParking,
    atFlexZoneFreeReturn,
    atFlexZoneChargeableReturn,
};

struct Area {
    QString id;
    QString provider;
    AreaType type;
    QString color;
    QList<QGeoCoordinate> polygon;

    explicit operator QVariant() const
    {
        QVariantMap m;
        m.insert("id", id);
        m.insert("provider", provider);
        m.insert("type", type);
        m.insert("color", color);

        QVariantList coords;

        for (const QGeoCoordinate& p : polygon) {
            QVariantList coordinate;
            coordinate << p.longitude();
            coordinate << p.latitude();
            QVariantMap map;
            map.insert("coordinate", coordinate);

            coords << map;
        }

        m.insert("polygon", coords);
        return m;
    }
};

using AreaList = QList<Area>;

inline QVariantList& operator<<(QVariantList& list, const AreaList& areas)
{
    for (const Area& a : areas)
        list << static_cast<QVariant>(a);

    return list;
}

struct Terminal {
    QString id;
    QString provider;
    QString name;
    QGeoCoordinate coordinate;
    int ridesTotal;
    int ridesAvailable;
    int racksTotal;
    int racksAvailable;
    bool maintenance;

    explicit operator QVariant() const
    {
        QVariantMap m;
        m.insert("id", id);
        m.insert("provider", provider);
        m.insert("name", name);
        m.insert("coordinate", QVariant::fromValue(coordinate));
        m.insert("ridesTotal", ridesTotal);
        m.insert("ridesAvailable", ridesAvailable);
        m.insert("racksTotal", racksTotal);
        m.insert("racksAvailable", racksAvailable);
        m.insert("maintenance", maintenance);

        return m;
    }
};

using TerminalList = QList<Terminal>;

inline QVariantList& operator<<(QVariantList& list, const TerminalList& terminals)
{
    for (const Terminal& a : terminals)
        list << static_cast<QVariant>(a);

    return list;
}

} // namespace scooter

#endif // TYPES_HPP
