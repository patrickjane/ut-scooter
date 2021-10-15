// **************************************************************************
// class ProviderBird
// 02.07.2021
// implementaton of Bird scooter REST API
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

#include <QDebug>
#include "providerbird.h"
#include "types.hpp"

#include <QSettings>
#include <QUuid>
#include <QDebug>
#include <QUrlQuery>
#include <QGeoPositionInfoSource>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#define LOGIN_URL       "https://api-auth.prod.birdapp.com/api/v1/auth/email"
#define CONFIRM_URL     "https://api-auth.prod.birdapp.com/api/v1/auth/magic-link/use"
#define PROFILE_URL     "https://api-bird.prod.birdapp.com/user"
#define REFRESH_URL     "https://api-auth.prod.birdapp.com/api/v1/auth/refresh/token"
#define SCOOTER_URL     "https://api-bird.prod.birdapp.com/bird/nearby"
#define ALARM_URL       "https://api-bird.prod.birdapp.com/bird/chirp"
#define AREAS_URL       "https://api.birdapp.com/area/nearby"
#define CONFIG_URL      "https://api.birdapp.com/config"
#define ACTIVE_RIDE_URL "https://api-bird.prod.birdapp.com/ride/active"
#define START_RIDE_URL  "https://api-bird.prod.birdapp.com/ride"
#define STOP_RIDE_URL   "https://api-bird.prod.birdapp.com/ride/complete"
#define PHOTO_URL       "https://api-bird.prod.birdapp.com/ride/photo"
#define SCAN_URL        "https://api-bird.prod.birdapp.com/scan"
#define VOUCHER_URL     "https://api-bird.prod.birdapp.com/coupon/promo"

//#define LOGIN_URL       "http://10.0.60.43:3000/login"
//#define CONFIRM_URL     "http://10.0.60.43:3000/confirm"
//#define PROFILE_URL     "http://10.0.60.43:3000/profile"
//#define REFRESH_URL     "http://10.0.60.43:3000/refresh"
//#define SCOOTER_URL     "http://10.0.60.43:3000/scooters"
//#define ALARM_URL       "http://10.0.60.43:3000/bird/chirp"
//#define AREAS_URL       "http://10.0.60.43:3000/areas"
//#define CONFIG_URL      "http://10.0.60.43:3000/config"
//#define ACTIVE_RIDE_URL "http://10.0.60.43:3000/ride/active"
//#define START_RIDE_URL  "http://10.0.60.43:3000/ride"
//#define STOP_RIDE_URL   "http://10.0.60.43:3000/ride/complete"
//#define PHOTO_URL       "http://10.0.60.43:3000/ride/photo"
//#define SCAN_URL        "http://10.0.60.43:3000/scan"
//#define VOUCHER_URL     "http://10.0.60.43:3000/coupon/promo"

namespace C {
#include <libintl.h>
}

// **************************************************************************
// namespace scooter
// **************************************************************************

namespace scooter
{
   // **************************************************************************
   // ctor/dtor
   // **************************************************************************

   ProviderBird::ProviderBird(network::Network* net, QGeoPositionInfoSource* position, QObject* parent)
      : Provider(net, position, parent)
   {
      QSettings settings;

      // get stuff from settings

      if (settings.contains("bird/deviceID"))
         deviceID = settings.value("bird/deviceID").toString();
      else
      {
         deviceID = QUuid::createUuid().toString().remove("{").remove("}").toUpper();
         settings.setValue("bird/deviceID", deviceID);
      }

      if (settings.contains("bird/account"))
         account = settings.value("bird/account").toString();

      if (settings.contains("bird/refreshToken"))
         refreshToken = settings.value("bird/refreshToken").toString();

      if (settings.contains("bird/priceInfo"))
      {
         auto info = settings.value("bird/priceInfo").toMap();

         priceInfo.basePrice = info["basePrice"].toDouble();
         priceInfo.pricePerMin = info["pricePerMin"].toDouble();
         priceInfo.priceString = info["priceString"].toString();
      }

      // prepare default HTTP headers for REST API

      defaultHeaders["User-Agent"] = "Bird/4.119.0(co.bird.Ride; build:3; iOS 14.3.0) Alamofire/5.2.2";
      defaultHeaders["Device-Id"] = deviceID;
      defaultHeaders["Platform"] = "ios";
      defaultHeaders["App-Version"] = "4.119.0";
      defaultHeaders["Content-Type"] = "application/json";
      defaultHeaders["legacyrequest"] = "false";
      defaultHeaders["App-Name"] = "Bird";

      connect(position, &QGeoPositionInfoSource::positionUpdated, this, [this, position](const QGeoPositionInfo& update)
      {
         if (isLoggedIn())
         {
            reloadConfig(update.coordinate());
            loadActiveRide(update.coordinate());
            disconnect(position, &QGeoPositionInfoSource::positionUpdated, this, 0);
         }
      });
   }

   // **************************************************************************
   // init
   // **************************************************************************

   void ProviderBird::init()
   {
      if (!refreshToken.isEmpty())
         refreshLogin();
      else
      {
         fReady = true;
         emit ready();
      }
   }

   // **************************************************************************
   // login
   // **************************************************************************

   void ProviderBird::login(QString loginId, QString)
   {
      network::ReqBody body;
      body["email"] = account = loginId;

      defaultHeaders.remove("Authorization");

      net->postJson<network::ReqCallback>(QUrl(LOGIN_URL), body, defaultHeaders, [this](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Login response: " + QString::number(code) + " (" + body + ")");
            return reportNetworkError(C::gettext("Login request failed"), err, code, body.isEmpty());
         }

         log(Severity::Info, "Login response: " + QString::number(code));

         auto doc = QJsonDocument::fromJson(body);
         auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (!parsed.isEmpty() && parsed.contains("validation_required") && parsed["validation_required"].toBool())
         {
            emit confirmLoginNeeded();
         }
         else if (parsed.contains("access") && parsed.contains("refresh"))
         {
            saveLogin(parsed);
            _reloadConfig();
         }
         else
         {
            emit error(C::gettext("Login failed (unexpected response)"));
         }
      });
   }

   // **************************************************************************
   // confirmLogin
   // **************************************************************************

   void ProviderBird::confirmLogin(QString confirmation)
   {
      network::ReqBody body;
      body["token"] = confirmation;

      net->postJson<network::ReqCallback>(QUrl(CONFIRM_URL), body, defaultHeaders, [this](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Confirm login response: " + QString::number(code) + " (" + body + ")");
            return reportNetworkError(C::gettext("Login confirmation failed"), err, code, body.isEmpty());
         }

         log(Severity::Info, "Confirm login response: " + QString::number(code));

         auto doc = QJsonDocument::fromJson(body);
         auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (parsed.isEmpty() || !parsed.contains("access") || !parsed.contains("refresh"))
         {
            emit error(C::gettext("Login failed (no tokens)"));
            return;
         }

         saveLogin(parsed);
         _reloadConfig();
      });
   }

   // **************************************************************************
   // logout
   // **************************************************************************

   void ProviderBird::logout()
   {
      clearLogin();
      Provider::logout();

      emit loginStatusChanged(false, "");
   }

   // **************************************************************************
   // refreshLogin
   // **************************************************************************

   void ProviderBird::refreshLogin()
   {
      defaultHeaders["Authorization"] = "Bearer " + refreshToken;

      net->postJson<network::ReqCallback>(QUrl(REFRESH_URL), network::ReqBody(), defaultHeaders, [this](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Refresh login response: " + QString::number(code) + " (" + body + ")");
            return reportNetworkError(C::gettext("Login refresh failed"), err, code, body.isEmpty());
         }

         log(Severity::Info, "Refresh login response: " + QString::number(code));

         auto doc = QJsonDocument::fromJson(body);
         auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (parsed.isEmpty() || !parsed.contains("access") || !parsed.contains("refresh"))
         {
            emit error(C::gettext("Login refresh failed (no tokens)"));
            return;
         }

         saveLogin(parsed);

         _reloadConfig();
      });
   }

   // **************************************************************************
   // getProfile
   // **************************************************************************

   void ProviderBird::getProfile()
   {
      if (!isLoggedIn())
      {
         emit error(C::gettext("Not logged in"));
         return;
      }

      network::ReqHeaders thisHeaders = defaultHeaders;

      if (!position || !position->lastKnownPosition().isValid())
      {
         emit error(C::gettext("Location not available, can not load profile"));
         return;
      }

      auto pos = position->lastKnownPosition().coordinate();

      thisHeaders["Location"] = QString("{\"latitude\":%1,\"longitude\":%2,\"altitude\":%3,\"accuracy\":65,\"speed\":-1,\"heading\":-1}")
            .arg(pos.latitude()).arg(pos.longitude()).arg("500");

      net->get<network::ReqCallback>(QUrl(PROFILE_URL), thisHeaders, [this](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
            return reportNetworkError(C::gettext("Loading profile failed"), err, code, body.isEmpty());

         auto doc = QJsonDocument::fromJson(body);
         auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         emit profile(parsed.toVariantMap());
      });
   }

   // **************************************************************************
   // _reloadConfig
   // **************************************************************************

   void ProviderBird::_reloadConfig()
   {
      if (!position)
         return;

      QGeoPositionInfo ifo = position->lastKnownPosition();

      if (!ifo.isValid())
         return;

      QGeoCoordinate pos = ifo.coordinate();

      reloadConfig(pos);
      loadActiveRide(pos);
   }

   // **************************************************************************
   // reloadConfig
   // **************************************************************************

   void ProviderBird::reloadConfig(QGeoCoordinate coordinates)
   {
      if (!isLoggedIn())
         return;

      QUrl url(CONFIG_URL);

      if (coordinates.isValid())
      {
         QUrlQuery query;
         query.addQueryItem("latitude", QString::number(coordinates.latitude()));
         query.addQueryItem("longitude", QString::number(coordinates.longitude()));
         query.addQueryItem("radius", "1000");
         url.setQuery(query.query());
      }

      net->get<network::ReqCallback>(url, defaultHeaders, [this, coordinates](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Reload config response: " + QString::number(code) + " (" + body + ")");
            return reportNetworkError(C::gettext("Loading configuration failed"), err, code, body.isEmpty());
         }

         log(Severity::Info, "Reload config response: " + QString::number(code));

         auto doc = QJsonDocument::fromJson(body);
         auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (parsed.isEmpty() || !parsed.contains("ride"))
         {
            qDebug() << "No price info in config found";
            return;
         }

         auto ride = parsed["ride"].toObject();

         if (ride.contains("base_price"))
            priceInfo.basePrice = ride["base_price"].toDouble() / 100;

         if (ride.contains("minute_price"))
            priceInfo.pricePerMin = ride["minute_price"].toDouble() / 100;

         if (ride.contains("localized_dynamic_price_string"))
            priceInfo.priceString = ride["localized_dynamic_price_string"].toString();

         QSettings settings;
         settings.setValue("bird/priceInfo", QVariant(priceInfo));

         fReady = true;
         emit ready();
         emit loginStatusChanged(true, account);
      });
   }

   // **************************************************************************
   // loadActiveRide
   // **************************************************************************

   void ProviderBird::loadActiveRide(QGeoCoordinate coordinates)
   {
      if (!isLoggedIn())
         return;

      network::ReqHeaders thisHeaders = defaultHeaders;
      auto locationHeader = QString("{\"latitude\":%1,\"longitude\":%2,\"altitude\":100,\"accuracy\":65,\"speed\":-1,\"heading\":-1}")
            .arg(coordinates.latitude()).arg(coordinates.longitude());
      thisHeaders["Location"] = locationHeader;

      net->get<network::ReqCallback>(QUrl(ACTIVE_RIDE_URL), thisHeaders, [this](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || (code != 204 && code != 200))
         {
            log(Severity::Error, "Load active ride response: " + QString::number(code) + " (" + body + ")");
            return reportNetworkError(C::gettext("Loading existing/active ride failed"), err, code, body.isEmpty());
         }

         log(Severity::Info, "Load active ride response: " + QString::number(code) + " Body: " + body);

         if (code == 204 || body.isEmpty())
            return;

         auto doc = QJsonDocument::fromJson(body);
         auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (parsed.isEmpty() || !parsed.contains("id") || !parsed.contains("started_at"))
         {
            emit error(C::gettext("Failed to load existing/active ride (missing information in response)"));
            return;
         }

         emit activeRideLoaded(parsed["id"].toString(), QDateTime::fromString(parsed["started_at"].toString(), Qt::ISODate));
      });
   }

   // **************************************************************************
   // reloadScooters
   // **************************************************************************

   void ProviderBird::reloadScooters(QGeoCoordinate coordinates, int radius, ResultCallback<ScooterMap> callback)
   {
      if (!isLoggedIn())
         return;

      network::ReqHeaders thisHeaders = defaultHeaders;
      thisHeaders["Location"] = QString("{\"latitude\":%1,\"longitude\":%2,\"altitude\":%3,\"accuracy\":65,\"speed\":-1,\"heading\":-1}")
            .arg(coordinates.latitude()).arg(coordinates.longitude()).arg(radius);

      QUrl url(SCOOTER_URL);
      QUrlQuery query;
      query.addQueryItem("latitude", QString::number(coordinates.latitude()));
      query.addQueryItem("longitude", QString::number(coordinates.longitude()));
      query.addQueryItem("radius", QString::number(radius));
      url.setQuery(query.query());

      net->get<network::ReqCallback>(url, thisHeaders, [this, callback](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Loading scooters response: " + QString::number(code) + " (" + body + ")");

            static QString prefix = C::gettext("Loading scooters failed");
            QString errStr = (prefix + " (%1/%2/%3)").arg(err).arg(code).arg(body.isEmpty());

            return callback(errStr, ScooterMap{});
         }

         log(Severity::Info, "Loading scooters response: " + QString::number(code));

         ScooterMap scooters;
         auto doc = QJsonDocument::fromJson(body);
         auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (parsed.isEmpty() || !parsed.contains("birds"))
         {
            qDebug() << "No bird scooters found";
            return callback("", scooters);
         }

         foreach (auto bird, parsed["birds"].toArray())
         {
            auto dict = bird.toObject();

            if (!dict.contains("id")
                || !dict.contains("location")
                || !dict.contains("battery_level")
                || !dict.contains("estimated_range")
                || !dict.contains("captive"))
            {
               qDebug() << "skipping invalid bird #1";
               continue;
            }

            auto loc = dict["location"].toObject();

            if (!loc.contains("latitude") || !loc.contains("longitude"))
            {
               qDebug() << "skipping invalid bird #2";
               continue;
            }

            QString mapId = dict["id"].toString();

            scooters[mapId] =
                  Scooter{
                  mapId,
                  "",            // unlockId
                  "",            // qrCode
                  kProviderBird,
                  QGeoCoordinate(loc["latitude"].toDouble(), loc["longitude"].toDouble()),
                  dict["battery_level"].toInt(),
                  dict["estimated_range"].toDouble(),
                  dict["captive"].toBool(),
                  true,          // available (info can only be retrieved via Scan-API-endpoint, not here)
                  priceInfo.basePrice,
                  priceInfo.pricePerMin,
                  priceInfo.priceString
            };
         }

         callback("", scooters);
      });
   }

   // **************************************************************************
   //  loadScooter
   // **************************************************************************

   void ProviderBird::reloadAreas(QGeoCoordinate coordinates, int radius)
   {
      if (!isLoggedIn())
         return;

      areas.clear();

      network::ReqHeaders thisHeaders = defaultHeaders;
      thisHeaders["Location"] = QString("{\"latitude\":%1,\"longitude\":%2,\"altitude\":%3,\"accuracy\":65,\"speed\":-1,\"heading\":-1}")
            .arg(coordinates.latitude()).arg(coordinates.longitude()).arg(radius);

      QUrl url(AREAS_URL);
      QUrlQuery query;
      query.addQueryItem("latitude", QString::number(coordinates.latitude()));
      query.addQueryItem("longitude", QString::number(coordinates.longitude()));
      query.addQueryItem("radius", QString::number(radius));
      url.setQuery(query.query());

      net->get<network::ReqCallback>(url, thisHeaders, [this](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Loading areas response: " + QString::number(code) + " (" + body + ")");
            emit areasChanged(areas);
            return reportNetworkError(C::gettext("Loading areas failed"), err, code, body.isEmpty());
         }

         log(Severity::Info, "Loading areas response: " + QString::number(code));

         auto doc = QJsonDocument::fromJson(body);
         QJsonArray parsed = (!doc.isNull() && doc.isArray()) ? doc.array() : QJsonArray();

         for (const QJsonValue& jsonArea : parsed)
         {
            if (!jsonArea.isObject())
            {
               qDebug() << "Invalid/unexpected data for no-parking zone #0";
               continue;
            }

            QJsonObject obj = jsonArea.toObject();

            if (!obj.contains("no_parking") || !obj["no_parking"].toBool() || !obj.contains("id") || !obj.contains("region"))
               continue;

            QJsonObject region = static_cast<QJsonValue>(obj["region"]).toObject(QJsonObject());

            if (!region.contains("rings") || !region["rings"].isArray())
            {
               qDebug() << "Invalid/unexpected data for no-parking zone #1";
               continue;
            }

            QJsonArray rings = static_cast<QJsonValue>(region["rings"]).toArray();

            if (!rings.size())
            {
               qDebug() << "Invalid/unexpected data for no-parking zone #2";
               continue;
            }

            QJsonObject pointsObj = static_cast<QJsonValue>(rings[0]).toObject(QJsonObject());

            if (!pointsObj.contains("points") || !pointsObj["points"].isArray())
            {
               qDebug() << "Invalid/unexpected data for no-parking zone #3";
               continue;
            }

            QList<QGeoCoordinate> coords;

            for (const QJsonValue& point : pointsObj["points"].toArray())
            {
               if (!point.isObject())
               {
                  qDebug() << "Invalid/unexpected data for no-parking zone #4";
                  coords.clear();
                  break;
               }

               auto p = point.toObject();

               if (!p.contains("latitude") || !p.contains("longitude"))
               {
                  qDebug() << "Invalid/unexpected data for no-parking zone #5";
                  coords.clear();
                  break;
               }

               coords << QGeoCoordinate(p["latitude"].toDouble(), p["longitude"].toDouble());
            }

            if (!coords.size())
            {
               qDebug() << "Invalid/unexpected data for no-parking zone #5";
               continue;
            }

            areas << Area{ obj["id"].toString(), AreaType::atNoParking, std::move(coords) };
         }

         emit areasChanged(areas);
      });
   }

   // **************************************************************************
   //  ringScooter
   // **************************************************************************

   void ProviderBird::ringScooter(QString id, QGeoCoordinate coordinate)
   {
      if (!isLoggedIn())
      {
         emit error(C::gettext("Not logged in"));
         return;
      }

      network::ReqHeaders thisHeaders = defaultHeaders;
      thisHeaders["Location"] = QString("{\"latitude\":%1,\"longitude\":%2}")
            .arg(coordinate.latitude()).arg(coordinate.longitude());

      network::ReqBody body;
      body["alarm"] = true;
      body["bird_id"] = id;

      qDebug() << "ProviderBird::ringScooter " << id;

      net->putJson<network::ReqCallback>(QUrl(ALARM_URL), body, thisHeaders, [this](int err, int code, QByteArray body)
      {
         qDebug() << "ProviderBird::ringScooter callback";
         qDebug() << "HTTP Code " << code << " Body: " << body;

         if (err != QNetworkReply::NoError || code != 200)
            return reportNetworkError(C::gettext("Alarming scooter failed"), err, code, body.isEmpty());

         emit notify(C::gettext("Scooter"), C::gettext("Scooter alarm successfully triggered"));
      });
   }

   // **************************************************************************
   //  scanScooter
   // **************************************************************************

   void ProviderBird::scanScooter(QString qrCode, QGeoCoordinate coordinate, ResultCallback<Scooter> callback)
   {
      if (!isLoggedIn())
      {
         callback(C::gettext("Not logged in"), Scooter{});
         return;
      }

      network::ReqHeaders thisHeaders = defaultHeaders;
      thisHeaders["Location"] = QString("{\"latitude\":%1,\"longitude\":%2}")
            .arg(coordinate.latitude()).arg(coordinate.longitude());

      network::ReqBody body;
      body["barcode"] = qrCode;
      body["mode"] = "rider";

      net->postJson<network::ReqCallback>(QUrl(SCAN_URL), body, thisHeaders, [this, callback, qrCode](int err, int code, QByteArray body)
      {
         qDebug() << "ProviderBird::loadScooter callback";
         qDebug() << "HTTP Code " << code << " Body: " << body;

         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Scan scooter response: " + QString::number(code) + " (" + body + ")");

            QString errStr = (QString(C::gettext("Starting ride failed")) + " (%1/%2/%3)").arg(err).arg(code).arg(body.isEmpty());
            callback(errStr, Scooter{});
            return;
         }

         log(Severity::Info, "Scan scooter response: " + QString::number(code) + " Body: " + body);

         auto doc = QJsonDocument::fromJson(body);
         auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (parsed.isEmpty() || !parsed.contains("bird_id") || !parsed.contains("status") || !parsed.contains("bird"))
         {
            callback(C::gettext("Scooter could not successfully be scanned / details could not be fetched"), Scooter{});
            return;
         }

         auto bird = parsed["bird"].toObject();

         if (!bird.contains("actual_battery_level") || !bird.contains("estimated_range") || !bird.contains("location"))
         {
            callback(C::gettext("Scooter could not successfully be scanned / details could not be fetched"), Scooter{});
            return;
         }

         auto loc = bird["location"].toObject();

         if (!loc.contains("latitude") || !loc.contains("longitude"))
         {
            callback(C::gettext("Scooter could not successfully be scanned / details could not be fetched"), Scooter{});
            return;
         }

         callback("", Scooter{
                     "",                                 // mapId
                     parsed["bird_id"].toString(),
                     qrCode,
                     kProviderBird,
                     QGeoCoordinate(loc["latitude"].toDouble(), loc["longitude"].toDouble()),
                     bird["battery_level"].toInt(),
                     bird["estimated_range"].toDouble(),
                     bird["captive"].toBool(),
                     parsed["status"].toString() == "available",
                     priceInfo.basePrice,
                     priceInfo.pricePerMin,
                     priceInfo.priceString
                  });
      });
   }

   // **************************************************************************
   // startRide
   // **************************************************************************

   void ProviderBird::startRide(QString unlockId, QGeoCoordinate coordinate, ResultCallback<QString> callback)
   {
      if (!isLoggedIn())
         return;

      qDebug() << "ProviderBird::startRide rideId " << unlockId;

      network::ReqHeaders thisHeaders = defaultHeaders;
      thisHeaders["Location"] = QString("{\"latitude\":%1,\"longitude\":%2}")
            .arg(coordinate.latitude()).arg(coordinate.longitude());

      network::ReqBody body;
      body["bird_id"] = unlockId;
      body["unlock"] = "true";

      net->postJson<network::ReqCallback>(QUrl(START_RIDE_URL), body, thisHeaders, [this, callback](int err, int code, QByteArray body)
      {
         qDebug() << "ProviderBird::startRide callback";
         qDebug() << "HTTP Code " << code << " Body: " << body;

         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Start ride response: " + QString::number(code) + " (" + body + ")");

            QString errStr = (QString(C::gettext("Starting ride failed")) + " (%1/%2/%3)").arg(err).arg(code).arg(body.isEmpty());
            callback(errStr, "");
            return;
         }

         log(Severity::Info, "Start ride response: " + QString::number(code) + " Body: " + body);

         auto doc = QJsonDocument::fromJson(body);
         QJsonObject parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (parsed.isEmpty() || !parsed.contains("id") || parsed["id"].toString().isEmpty())
         {
            callback(C::gettext("Unlocking failed. Response did not contain ride information. Please re-check ride with official bird app. Issues in response:\n"), "");
            return;
         }

         if (parsed.contains("issues") && parsed["issues"].toArray().size())
         {
            static QString prefix = C::gettext("Problems while unlocking. Please re-check ride with official bird app.");

            callback(prefix + QJsonDocument(parsed["issues"].toArray()).toJson(), "");
            return;
         }

         callback("", parsed["id"].toString());
      });
   }

   // **************************************************************************
   // stopRide
   // **************************************************************************

   void ProviderBird::stopRide(QString rideId, QGeoCoordinate coordinate, SupplementaryCallback<QString> callback)
   {
      if (!isLoggedIn())
         return;

      qDebug() << "ProviderBird::stopRide: " << rideId << " " << coordinate;

      network::ReqHeaders thisHeaders = defaultHeaders;
      thisHeaders["Location"] = QString("{\"latitude\":%1,\"longitude\":%2}")
            .arg(coordinate.latitude()).arg(coordinate.longitude());

      network::ReqBody body;
      body["ride_id"] = rideId;

      net->putJson<network::ReqCallback>(QUrl(STOP_RIDE_URL), body, thisHeaders, [this, callback, thisHeaders, rideId](int err, int code, QByteArray body)
      {
         qDebug() << "ProviderBird::stopRide callback";
         qDebug() << "HTTP Code " << code << " Body: " << body;

         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Stop ride response: " + QString::number(code) + " (" + body + ")");

            QString errStr = (QString(C::gettext("Stopping ride failed")) + " (%1/%2/%3)").arg(err).arg(code).arg(body.isEmpty());
            callback(errStr, "", "");
            return;
         }

         log(Severity::Info, "Stop ride response: " + QString::number(code) + " Body: " + body);

         auto doc = QJsonDocument::fromJson(body);
         QJsonObject parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

         if (parsed.contains("issues") && parsed["issues"].toArray().size())
         {
            static QString prefix = C::gettext("Problems while stopping ride. Please re-check ride with official bird app. Issues in response:\n");

            callback(prefix + QJsonDocument(parsed["issues"].toArray()).toJson(), "", "");
            return;
         }

         if (parsed.isEmpty() || !parsed.contains("ride") || !parsed["ride"].isObject())
         {
            callback(C::gettext("Stopping ride failed. Response did not contain expected information. Please check with official bird app, or please check no-parking zone."), "", "");
            return;
         }

         auto ride = parsed["ride"].toObject();

         if (!ride.contains("completed_at"))
         {
            callback(C::gettext("Stopping ride failed. Response did not contain completed stamp. Please check with official bird app, or please check no-parking zone."), "", "");
            return;
         }

         QJsonObject infoObject;
         infoObject["provider"] = "bird";
         infoObject["completedAt"] = ride["completed_at"];

         if  (parsed.contains("distance"))
              infoObject["distance"] = parsed["distance"];

         if  (parsed.contains("duration"))
            infoObject["duration"] = parsed["duration"];

         if  (parsed.contains("cost"))
            infoObject["cost"] = parsed["cost"];

         if (parsed.contains("cost_without_coupon") && parsed["cost"].toString() != parsed["cost_without_coupon"].toString())
            infoObject["costRegular"] = parsed["cost_without_coupon"];

         if (ride.contains("canceled_at"))
         {
            infoObject["cancelledAt"] = ride["canceled_at"];

            callback("", QJsonDocument(infoObject).toJson(), "");
            return;
         }

         network::ReqBody putBody;
         putBody["ride_id"] = rideId;
         putBody["photo_url"] = "https://s3.amazonaws.com/bird-uploads-production/ride-photos/b64bacac-f2d6-40ad-85ba-ccb046264904.png";

         net->putJson<network::ReqCallback>(QUrl(PHOTO_URL), putBody, thisHeaders, [this, callback, infoObject](int err, int code, QByteArray body)
         {
            qDebug() << "ProviderBird::stopRide Photo upload callback";
            qDebug() << "HTTP Code " << code << " Body: " << body;

            if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
            {
               log(Severity::Error, "Photo upload response: " + QString::number(code) + " (" + body + ")");

               QString errStr = (QString(C::gettext("Uploading photo failed")) + " (%1/%2/%3)").arg(err).arg(code).arg(body.isEmpty());
               callback("", "", errStr);
               return;
            }

            log(Severity::Info, "Photo upload response: " + QString::number(code) + " Body: " + body);

            auto doc = QJsonDocument::fromJson(body);
            QJsonObject parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

            if (parsed.contains("issues") && parsed["issues"].toArray().size())
            {
               static QString prefix = C::gettext("Problems while uploading photo. Please re-check ride with official bird app. Issues in response:\n");

               callback("", QJsonDocument(infoObject).toJson(), prefix + QJsonDocument(parsed["issues"].toArray()).toJson());
               return;
            }

            callback("", QJsonDocument(infoObject).toJson(), "");
         });
      });
   }

   // **************************************************************************
   // addVoucherCode
   // **************************************************************************

   void ProviderBird::addVoucherCode(QString code)
   {
      if (!isLoggedIn())
         return;

      network::ReqBody body;
      body["link_code"] = code;

      net->postJson<network::ReqCallback>(QUrl(VOUCHER_URL), body, defaultHeaders, [this](int err, int code, QByteArray body)
      {
         if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
         {
            log(Severity::Error, "Add voucher code response: " + QString::number(code) + " (" + body + ")");
            return reportNetworkError(C::gettext("Adding voucher code failed"), err, code, body.isEmpty());
         }

         log(Severity::Info, "Add voucher code response: " + QString::number(code) + " Body: " + body);
      });
   }

   // **************************************************************************
   // saveLogin
   // **************************************************************************

   void ProviderBird::saveLogin(const QJsonObject& params)
   {
      accessToken = params["access"].toString();
      refreshToken = params["refresh"].toString();
      defaultHeaders["Authorization"] = "Bearer " + accessToken;

      QSettings settings;
      settings.setValue("bird/account", account);
      settings.setValue("bird/accessToken", accessToken);
      settings.setValue("bird/refreshToken", refreshToken);
   }

   // **************************************************************************
   // clearLogin
   // **************************************************************************

   void ProviderBird::clearLogin()
   {
      QSettings settings;
      settings.remove("bird/account");
      settings.remove("bird/accessToken");
      settings.remove("bird/refreshToken");
      settings.remove("bird/priceInfo");

      defaultHeaders.remove("Authorization");

      accessToken.clear();
      refreshToken.clear();
   }
}
