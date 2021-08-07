// **************************************************************************
// class Scooter
// 02.07.2021
// Utility classes
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

#ifndef TYPES_HPP
#define TYPES_HPP

#include <map>
#include <functional>
#include <QString>
#include <QGeoCoordinate>
#include <QDateTime>
#include <QDebug>

// **************************************************************************
// namespace scooter
// **************************************************************************

namespace scooter
{
   const QString kProviderBird = "bird";
   const QString kProviderVoi = "voi";
   const QString kProviderLime = "lime";
   const QString kProviderBolt = "bolt";
   const QString kProviderTier = "tier";

   template<typename T>
   using ResultCallback = std::function<void(QString, T)>;

   template<typename T>
   using SupplementaryCallback = std::function<void(QString, T, QString)>;

   struct Scooter
   {
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

            return m;
         }
   };

   using ScooterMap = std::map<QString, Scooter>;
   using ScooterRegistry = std::map<QString, ScooterMap>;

   inline QVariantList& operator<<(QVariantList& list, const Scooter& scooter)
   {
      return list << static_cast<QVariant>(scooter);
   }

   inline QVariantList& operator<<(QVariantList& list, const ScooterMap& map)
   {
      for (auto const& x : map)
         list << x.second;

      return list;
   }

   inline QVariantList& operator<<(QVariantList& list, const ScooterRegistry& registry)
   {
      for (auto const& x : registry)
         list << x.second;

      return list;
   }

   struct ActiveRide
   {
         QString rideId;
         QString provider;
         QDateTime started;

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("rideId", rideId);
            m.insert("provider", provider);
            m.insert("started", QVariant(started));
            return m;
         }
   };

   struct PriceInfo
   {
         double basePrice;
         double pricePerMin;
         QString priceString;

         void clear() { basePrice = pricePerMin = 0.0; priceString.clear(); }

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("basePrice", basePrice);
            m.insert("pricePerMin", pricePerMin);
            m.insert("priceString", priceString);
            return m;
         }
   };

   enum AreaType
   {
      atNoParking
   };

   struct Area
   {
         QString id;
         AreaType type;
         QList<QGeoCoordinate> polygon;

         explicit operator QVariant() const
         {
            QVariantMap m;
            m.insert("id", id);
            m.insert("type", type);

            QVariantList coords;

            for (const QGeoCoordinate& p : polygon)
            {
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
}

#endif // TYPES_HPP
