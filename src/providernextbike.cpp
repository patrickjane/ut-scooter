// **************************************************************************
// class ProviderNextbike
// implementaton of NextBike REST API
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

#include <iostream>
#include <map>
#include <set>

#include "providernextbike.h"
#include "scooter_private.h"
#include "types.hpp"
#include <QDebug>

#include <QDebug>
#include <QGeoPositionInfoSource>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QSettings>
#include <QUrlQuery>
#include <QUuid>

#define LOGIN_URL "https://api.nextbike.net/api/login.json"
#define ZONES_URL "https://api.nextbike.net/api/v1.1/getFlexzones.json"
#define TERMINALS_URL "https://api.nextbike.net/maps/nextbike-live.flatjson"
#define ACTIVE_RIDE_URL "https://api.nextbike.net/api/getOpenRentals.json"
#define PROFILE_URL "https://api.nextbike.net/api/list.json"
#define SCAN_URL "https://api.nextbike.net/api/getBikeState.json"
#define UNLOCK_URL "https://api.nextbike.net/api/rent.json"
#define RIDE_DETAILS_URL "https://api.nextbike.net/api/getRentalDetails.json"
#define ACCOUN_HISTORY_URL "https://api.nextbike.net/api/list.json"

// #define LOGIN_URL "http://10.0.60.43:3003/login"
// #define ZONES_URL "http://10.0.60.43:3003/zones"
// #define TERMINALS_URL "http://10.0.60.43:3003/terminals"
// #define ACTIVE_RIDE_URL "http://10.0.60.43:3003/activeride"
// #define PROFILE_URL "http://10.0.60.43:3003/list"
// #define SCAN_URL "http://10.0.60.43:3003/scan"
// #define UNLOCK_URL "http://10.0.60.43:3003/unlock"
// #define RIDE_DETAILS_URL "http://10.0.60.43:3003/rideDetails"
// #define ACCOUN_HISTORY_URL "http://10.0.60.43:3003/list"

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

ProviderNextbike::ProviderNextbike(network::Network* net, QGeoPositionInfoSource* position,
                                   QObject* parent)
  : Provider(net, position, parent)
{
    QSettings settings;

    // get stuff from settings

    if (settings.contains("nextbike/country"))
        country = settings.value("nextbike/country").toString();

    if (settings.contains("nextbike/city"))
        city = settings.value("nextbike/city").toString();

    if (settings.contains("nextbike/domain"))
        domain = settings.value("nextbike/domain").toString();

    if (settings.contains("nextbike/account"))
        account = settings.value("nextbike/account").toString();

    if (settings.contains("nextbike/loginKey"))
        loginKey = settings.value("nextbike/loginKey").toString();

    if (settings.contains("nextbike/currency"))
        currency = settings.value("nextbike/currency").toString();
    else
        currency = "EUR";

    defaultHeaders["Content-Type"] = "application/json";
}

// **************************************************************************
// init
// **************************************************************************

void ProviderNextbike::init()
{
    if (isLoggedIn()) {
        loadActiveRide(QGeoCoordinate());
    }

    fReady = true;
    emit ready();
}

// **************************************************************************
// login
// **************************************************************************

void ProviderNextbike::login(QString loginId, QString password)
{
    network::ReqBody body;
    body["apikey"] = ScooterPrivate::getApiKey("nextbike");
    body["mobile"] = loginId;
    body["pin"] = password;
    body["show_errors"] = "true";

    net->postJson<network::ReqCallback>(
      QUrl(LOGIN_URL), body, defaultHeaders, [this, loginId](int err, int code, QByteArray body) {
          qDebug() << "LOGIN RESPONSE: " << code << err << body;

          if (err != QNetworkReply::NoError || code != 200 || body.isEmpty()) {
              log(Severity::Error, "Login response: " + QString::number(code) + " (" + body + ")");
              return reportNetworkError(C::gettext("Login request failed"), err, code,
                                        body.isEmpty());
          }

          log(Severity::Info, "Login response: " + QString::number(code));

          auto doc = QJsonDocument::fromJson(body);
          auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

          if (parsed.isEmpty() || !parsed.contains("user") || !parsed["user"].isObject()) {
              if (parsed.contains("error") && parsed["error"].isObject()
                  && parsed["error"].toObject().contains("message")) {
                  emit error(parsed["error"].toObject()["message"].toString());
              } else {
                  emit error(C::gettext("Login failed (unexpected response)"));
              }

              return;
          }

          auto user = parsed["user"].toObject();

          if (!user.contains("loginkey") || !user["loginkey"].isString()) {
              emit error(C::gettext("Login failed (no login key received)"));
              return;
          }

          account = loginId;

          saveLogin(account, user["loginkey"].toString());
          emit loginStatusChanged(true, account);
      });
}

// **************************************************************************
// logout
// **************************************************************************

void ProviderNextbike::logout()
{
    clearLogin();
    Provider::logout();

    emit loginStatusChanged(false, "");
}

// **************************************************************************
// getProfile
// **************************************************************************

void ProviderNextbike::getProfile()
{
    if (!isLoggedIn()) {
        emit error(C::gettext("Not logged in"));
        return;
    }

    QUrl url(PROFILE_URL);
    QUrlQuery query;
    query.addQueryItem("apikey", ScooterPrivate::getApiKey("nextbike"));
    query.addQueryItem("loginkey", loginKey);
    query.addQueryItem("show_errors", "true");
    query.addQueryItem("language", QLocale::system().name().split("_").first());
    url.setQuery(query.query());

    network::ReqBody body;
    body["bookings"] = true;

    net->postJson<network::ReqCallback>(
      url, body, defaultHeaders, [this](int err, int code, QByteArray body) {
          if (err != QNetworkReply::NoError || code != 200 || body.isEmpty())
              return reportNetworkError(C::gettext("Loading profile failed"), err, code,
                                        body.isEmpty());

          auto doc = QJsonDocument::fromJson(body);
          auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

          if (parsed.isEmpty() || !parsed.contains("user") || !parsed["user"].isObject()) {
              emit error(C::gettext("Loading profile failed (empty/malformed response)"));
              return;
          }

          auto user = parsed["user"].toObject();

          QVariantList items;

          if (user.contains("mobile")) {
              QVariantMap m;
              m.insert("id", C::gettext("Phone"));
              m.insert("value", user["mobile"].toString());

              items << m;
          }

          if (user.contains("email")) {
              QVariantMap m;
              m.insert("id", C::gettext("E-Mail"));
              m.insert("value", user["email"].toString());

              items << m;
          }

          if (user.contains("lang")) {
              QVariantMap m;
              m.insert("id", C::gettext("Language"));
              m.insert("value", user["lang"].toString());

              items << m;
          }

          if (user.contains("currency")) {
              QVariantMap m;
              m.insert("id", C::gettext("Currency"));
              m.insert("value", user["currency"].toString());

              currency = user["currency"].toString();

              QSettings settings;
              settings.setValue("nextbike/currency", currency);

              items << m;
          }

          if (user.contains("credits")) {
              QVariantMap m;
              m.insert("id", C::gettext("Credits"));
              m.insert("value", QString::number(user["credits"].toDouble() / 100.0, 'f', 2));

              items << m;
          }

          if (user.contains("free_seconds")) {
              QVariantMap m;
              m.insert("id", C::gettext("Free seconds"));
              m.insert("value", user["free_seconds"].toInt());

              items << m;
          }

          emit profile(items);
      });
}

// **************************************************************************
// getAccountHistory
// **************************************************************************

void ProviderNextbike::getAccountHistory(ResultCallback<QString> callback)
{
    if (!isLoggedIn()) {
        emit error(C::gettext("Not logged in"));
        return;
    }

    QUrl url(ACCOUN_HISTORY_URL);
    QUrlQuery query;
    query.addQueryItem("apikey", ScooterPrivate::getApiKey("nextbike"));
    query.addQueryItem("loginkey", loginKey);
    query.addQueryItem("show_errors", "true");
    query.addQueryItem("language", QLocale::system().name().split("_").first());
    url.setQuery(query.query());

    network::ReqBody body;
    body["bookings"] = true;

    net->postJson<network::ReqCallback>(
      url, body, defaultHeaders, [this, callback](int err, int code, QByteArray body) {
          if (err != QNetworkReply::NoError || code != 200 || body.isEmpty()) {
              log(Severity::Error,
                  "Account history response: " + QString::number(code) + " (" + body + ")");

              QString errStr
                = (QString(C::gettext("Loading account history failed")) + " (%1/%2/%3)")
                    .arg(err)
                    .arg(code)
                    .arg(body.isEmpty());
              callback(errStr, "");
              return;
          }

          log(Severity::Info,
              "Account history response: " + QString::number(code) + " Body: " + body);

          auto doc = QJsonDocument::fromJson(body);
          auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

          if (parsed.isEmpty() || !parsed.contains("account")) {
              callback(
                C::gettext(
                  "Scooter could not successfully be scanned / details could not be fetched"),
                "");
              return;
          }

          auto account = parsed["account"].toObject();
          auto user = parsed["user"].toObject();

          if (!account.contains("items") || account["items"].toArray().size() == 0) {
              return callback("", "");
          }

          QJsonArray res;

          auto items = account["items"].toArray();

          for (const QJsonValue& item : items) {
              auto type = item["node"].toString();

              if (type != "rental" && type != "payment") {
                  continue;
              }

              QJsonObject obj;
              obj["type"] = type == "rental" ? C::gettext("Ride") : C::gettext("Payment");

              if (type == "rental") {
                  obj["date"] = item["start_time"];
                  obj["text"] = item["city"].toString() + ", " + QString(C::gettext("from")) + ": "
                                + item["start_place_name"].toString() + " "
                                + QString(C::gettext("to")) + " "
                                + item["end_place_name"].toString();

                  obj["cost"] = (item["price"].toInt() + item["price_service"].toInt());
                  obj["duration"] = item["end_time"].toInt() - item["start_time"].toInt();
              } else {
                  obj["date"] = item["date"].toString().toInt();
                  obj["text"] = item["text"];
                  obj["cost"] = item["amount"];
              }

              obj["locale"] = QLocale::system().name().split("_").first();
              obj["currency"] = user["currency"];

              res.append(obj);
          }

          callback("", QJsonDocument(res).toJson(QJsonDocument::Compact));
      });
}

// **************************************************************************
// setCountry
// **************************************************************************

void ProviderNextbike::setCountry(QString name)
{
    country = name;

    QSettings settings;
    settings.setValue("nextbike/country", country);

    qDebug() << "Saving new country: " << name;
}

// **************************************************************************
// setCity
// **************************************************************************

void ProviderNextbike::setCity(QString cityName, QString dom)
{
    domain = dom;
    city = cityName;

    QSettings settings;
    settings.setValue("nextbike/city", city);
    settings.setValue("nextbike/domain", domain);

    qDebug() << "Saving new city: " << cityName << dom;
}

// **************************************************************************
// loadActiveRide
// **************************************************************************

void ProviderNextbike::loadActiveRide(QGeoCoordinate coordinates)
{
    if (!isLoggedIn())
        return;

    QUrl url(ACTIVE_RIDE_URL);
    QUrlQuery query;
    query.addQueryItem("apikey", ScooterPrivate::getApiKey("nextbike"));
    query.addQueryItem("loginkey", loginKey);
    query.addQueryItem("show_errors", "true");
    url.setQuery(query.query());

    net->get<network::ReqCallback>(url, defaultHeaders, [this](int err, int code, QByteArray body) {
        if (err != QNetworkReply::NoError || code != 200) {
            log(Severity::Error,
                "Load active ride response: " + QString::number(code) + " (" + body + ")");
            return reportNetworkError(C::gettext("Loading existing/active ride failed"), err, code,
                                      body.isEmpty());
        }

        log(Severity::Info,
            "Load active ride response: " + QString::number(code) + " Body: " + body);

        if (body.isEmpty())
            return;

        auto doc = QJsonDocument::fromJson(body);
        auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

        if (parsed.isEmpty() || !parsed.contains("rentalCollection")
            || !parsed["rentalCollection"].isArray()) {
            emit error(
              C::gettext("Failed to load existing/active ride (missing information in response)"));
            return;
        }

        auto rentals = parsed["rentalCollection"].toArray();

        if (!rentals.size())
            return;

        auto activeRent = rentals[0].toObject();

        if (activeRent.isEmpty() || !activeRent.contains("id") || !activeRent.contains("bike")
            || !activeRent.contains("start_time")) {
            emit error(C::gettext(
              "Failed to load existing/active ride (missing information in response/rental list)"));
            return;
        }

        emit activeRideLoaded(QString::number(activeRent["id"].toInt()),
                              QDateTime::fromSecsSinceEpoch(activeRent["start_time"].toInt()),
                              activeRent["bike"].toString());
    });
}

// **************************************************************************
// reloadScooters
// **************************************************************************

void ProviderNextbike::reloadScooters(QGeoCoordinate coordinates, int radius)
{
    if (!isLoggedIn())
        return;

    terminals.clear();

    QUrl url(TERMINALS_URL);
    QUrlQuery query;
    query.addQueryItem("apikey", ScooterPrivate::getApiKey("nextbike"));
    query.addQueryItem("loginkey", loginKey);
    query.addQueryItem("lat", QString::number(coordinates.latitude()));
    query.addQueryItem("lng", QString::number(coordinates.longitude()));
    query.addQueryItem("distance", QString::number(radius));

    query.addQueryItem("show_errors", "true");
    url.setQuery(query.query());

    net->get<network::ReqCallback>(url, defaultHeaders, [this](int err, int code, QByteArray body) {
        if (err != QNetworkReply::NoError || code != 200 || body.isEmpty()) {
            if (code == 0) {
                log(Severity::Warning, "Loading scooters EMPTY response: " + QString::number(code)
                                         + " (" + body + ")");
                return;
            }

            log(Severity::Error,
                "Loading scooters response: " + QString::number(code) + " (" + body + ")");
            emit areasChanged(areas);
            return reportNetworkError(C::gettext("Loading scooters failed"), err, code,
                                      body.isEmpty());
        }

        log(Severity::Info,
            "Loading scooters response: " + QString::number(code) + " Body: " + body);

        ScooterList scooters;
        auto doc = QJsonDocument::fromJson(body);
        auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

        if (parsed.isEmpty() || !parsed.contains("cities") || !parsed["cities"].isArray()
            || !parsed.contains("places") || !parsed["places"].isArray()) {
            return reportNetworkError(C::gettext("Loading scooters failed"), 0, 0, false);
        }

        auto cities = parsed["cities"].toArray();
        auto places = parsed["places"].toArray();
        auto city = cities.first().toObject();

        // extract domain info from city only, ignore rest of cities/countries

        if (city.isEmpty() || !city.contains("domain") || !city["domain"].isString()) {
            return reportNetworkError(C::gettext("Loading scooters failed"), 0, 0, false);
        }

        QSettings settings;
        domain = city["domain"].toString();
        settings.setValue("nextbike/domain", domain);

        QMap<QString, QGeoCoordinate> bikePositionCache;

        // parse stations, ignoring stations with bike: true (will be parsed via bikes-array)

        for (const QJsonValue& obj : places) {
            if (!obj.isObject()) {
                qDebug() << "Invalid/unexpected data for place #0";
                continue;
            }

            auto place = obj.toObject();

            if (!place.contains("uid") || !place.contains("lat") || !place.contains("lng")
                || !place.contains("name") || !place.contains("bikes")
                || !place.contains("bikes_available_to_rent") || !place.contains("bike_racks")
                || !place.contains("free_racks") || !place.contains("maintenance")
                || !place.contains("bike") || !obj["uid"].isDouble() || !obj["lat"].isDouble()
                || !obj["lng"].isDouble() || !obj["name"].isString() || !obj["bikes"].isDouble()
                || !obj["bikes_available_to_rent"].isDouble() || !obj["bike_racks"].isDouble()
                || !obj["free_racks"].isDouble() || !obj["maintenance"].isBool()
                || !obj["bike"].isBool()) {
                qDebug() << "Invalid/unexpected data for place #1";
                continue;
            }

            auto uid = place["uid"].toString();
            auto name = place["name"].toString();
            auto coord = QGeoCoordinate(place["lat"].toDouble(), place["lng"].toDouble());
            auto bikes = place["bikes"].toInt();
            auto bikesAvail = place["bikes_available_to_rent"].toInt();
            auto racks = place["bike_racks"].toInt();
            auto racksAvail = place["free_racks"].toInt();
            auto maintenance = place["maintenance"].toBool();

            if (place["bike"].toBool()) {
                bikePositionCache[name.replace("BIKE ", "")] = coord;
            } else {
                terminals << Terminal {uid,        kProviderNextbike, name,  coord,
                                       bikes,      bikesAvail,        racks, racksAvail,
                                       maintenance};
            }
        }

        // parse bikes from bikes array

        if (parsed.contains("bikes") && parsed["bikes"].isArray()) {
            for (const QJsonValue& obj : parsed["bikes"].toArray()) {
                if (!obj.isObject()) {
                    qDebug() << "Invalid/unexpected data for bike #0";
                    continue;
                }

                auto bike = obj.toObject();

                if (!bike.contains("number") || !bike.contains("active")
                    || !bike["number"].isString() || !bike["active"].isBool()) {
                    qDebug() << "Invalid/unexpected data for bike #1";
                    continue;
                }

                auto number = bike["number"].toString();
                auto active = bike["active"].toBool();

                if (!bikePositionCache.contains(number)) {
                    //   qDebug() << "No cached position for bike '" << number << "'";
                    continue;
                }

                scooters << Scooter {number,
                                     number,
                                     "", // qrCode
                                     kProviderNextbike,
                                     bikePositionCache[number],
                                     -1,
                                     -1.0,
                                     false,
                                     active,
                                     -1.0,
                                     -1.0,
                                     ""};
            }
        }

        emit scootersChanged(scooters);
        emit terminalsChanged(terminals);
    });
}

// **************************************************************************
//  reloadAreas
// **************************************************************************

void ProviderNextbike::reloadAreas(QGeoCoordinate coordinates, int radius)
{
    if (!isLoggedIn())
        return;

    areas.clear();

    QUrl url(ZONES_URL);
    QUrlQuery query;
    query.addQueryItem("apikey", ScooterPrivate::getApiKey("nextbike"));
    query.addQueryItem("loginkey", loginKey);
    query.addQueryItem("show_errors", "true");
    url.setQuery(query.query());

    network::ReqBody body;
    body["domain"] = domain;

    net->postJson<network::ReqCallback>(
      url, body, defaultHeaders, [this](int err, int code, QByteArray body) {
          if (err != QNetworkReply::NoError || code != 200 || body.isEmpty()) {
              log(Severity::Error,
                  "Loading areas response: " + QString::number(code) + " (" + body + ")");
              emit areasChanged(areas);
              return reportNetworkError(C::gettext("Loading areas failed"), err, code,
                                        body.isEmpty());
          }

          log(Severity::Info, "Loading areas response: " + QString::number(code));

          auto doc = QJsonDocument::fromJson(body);
          auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

          if (parsed.isEmpty() || !parsed.contains("geojson") || !parsed["geojson"].isObject()) {
              qDebug() << "No geojson in nextbike flex zones response found";
              return;
          }

          auto geojson = parsed["geojson"].toObject();

          if (!geojson.contains("nodeValue") || !geojson["nodeValue"].isObject()) {
              qDebug() << "No nodeValue in geojson in nextbike flex zones response found";
              return;
          }

          auto nodeValue = geojson["nodeValue"].toObject();

          if (!nodeValue.contains("features") || !nodeValue["features"].isArray()) {
              qDebug() << "No features in geojson in nextbike flex zones response found";
              return;
          }

          auto features = nodeValue["features"].toArray();

          for (const QJsonValue& jsonArea : features) {
              if (!jsonArea.isObject()) {
                  qDebug() << "Invalid/unexpected data for flexzone #0";
                  continue;
              }

              QJsonObject obj = jsonArea.toObject();

              if (!obj.contains("geometry") || !obj.contains("properties")
                  || !obj["geometry"].isObject() || !obj["properties"].isObject()) {
                  qDebug() << "Invalid/unexpected data for flexzone #1";
                  continue;
              }

              // parse flexzone properties

              auto properties = obj["properties"].toObject();
              auto geometry = obj["geometry"].toObject();

              if (!geometry.contains("coordinates") || !properties.contains("fill")
                  || !properties.contains("flexzoneId") || !properties.contains("category")
                  || !geometry["coordinates"].isArray() || !properties["fill"].isString()
                  || !properties["category"].isString() || !properties["flexzoneId"].isString()) {
                  qDebug() << "Invalid/unexpected data for flexzone #2";
                  continue;
              }

              auto fill = properties["fill"].toString();
              auto category = properties["category"].toString();
              auto flexzoneId = properties["flexzoneId"].toString();

              // parse flexzone polygon

              QList<QGeoCoordinate> coords;
              auto coordinates = geometry["coordinates"].toArray();

              if (coordinates.size() <= 0) {
                  qDebug() << "Invalid/unexpected data for flexzone #3";
                  continue;
              }

              auto coordinates0 = coordinates.first();

              if (!coordinates0.isArray() || coordinates0.toArray().size() <= 0) {
                  qDebug() << "Invalid/unexpected data for flexzone #4";
                  continue;
              }

              for (const QJsonValue& point : coordinates0.toArray()) {
                  if (!point.isArray() || point.toArray().size() != 2) {
                      qDebug() << "Invalid/unexpected data for flexzone #5";
                      coords.clear();
                      break;
                  }

                  auto lat = point.toArray().last();
                  auto lon = point.toArray().first();

                  if (!lat.isDouble() || !lon.isDouble()) {
                      qDebug() << "Invalid/unexpected data for flexzone #6";
                      coords.clear();
                      break;
                  }

                  coords << QGeoCoordinate(lat.toDouble(), lon.toDouble());
              }

              // append to list

              if (category == "free_return")
                  areas << Area {flexzoneId, kProviderNextbike, AreaType::atFlexZoneFreeReturn,
                                 fill, std::move(coords)};
              else if (category == "chargeable_return")
                  areas << Area {flexzoneId, kProviderNextbike,
                                 AreaType::atFlexZoneChargeableReturn, fill, std::move(coords)};
              else
                  qDebug() << "Invalid/unexpected flexzone category: " << category;
          }

          emit areasChanged(areas);
      });
}

// **************************************************************************
//  scanScooter
// **************************************************************************

void ProviderNextbike::scanScooter(QString qrCode, QGeoCoordinate coordinate,
                                   ResultCallback<Scooter> callback)
{
    if (!isLoggedIn()) {
        callback(C::gettext("Not logged in"), Scooter {});
        return;
    }

    QUrl url(SCAN_URL);
    QUrlQuery query;
    query.addQueryItem("apikey", ScooterPrivate::getApiKey("nextbike"));
    query.addQueryItem("loginkey", loginKey);
    query.addQueryItem("show_errors", "true");
    url.setQuery(query.query());

    network::ReqBody body;
    body["bike"] = qrCode;

    net->postJson<network::ReqCallback>(
      url, body, defaultHeaders,
      [this, coordinate, qrCode, callback](int err, int code, QByteArray body) {
          qDebug() << "ProviderNextbike::loadScooter callback";
          qDebug() << "HTTP Code " << code << " Body: " << body;

          if (err != QNetworkReply::NoError || code != 200 || body.isEmpty()) {
              log(Severity::Error,
                  "Scan scooter response: " + QString::number(code) + " (" + body + ")");

              QString errStr = (QString(C::gettext("Starting ride failed")) + " (%1/%2/%3)")
                                 .arg(err)
                                 .arg(code)
                                 .arg(body.isEmpty());
              callback(errStr, Scooter {});
              return;
          }

          log(Severity::Info, "Scan scooter response: " + QString::number(code) + " Body: " + body);

          auto doc = QJsonDocument::fromJson(body);
          auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

          if (parsed.isEmpty() || !parsed.contains("bike")) {
              callback(
                C::gettext(
                  "Scooter could not successfully be scanned / details could not be fetched"),
                Scooter {});
              return;
          }

          auto bike = parsed["bike"].toObject();

          if (bike.isEmpty() || !bike.contains("pedelec_battery") || !bike.contains("state")
              || !bike.contains("state_description") || !bike.contains("active")) {
              callback(
                C::gettext(
                  "Scooter could not successfully be scanned / details could not be fetched"),
                Scooter {});
              return;
          }

          callback("",
                   Scooter {qrCode, qrCode, qrCode, kProviderNextbike, coordinate,
                            bike["pedelec_battery"].isNull() ? -1 : bike["pedelec_battery"].toInt(),
                            -1.0, !bike["active"].toBool(), bike["state"].toString() == "ok", -1.0,
                            -1.0, "", bike["state_description"].toString()});
      });
}

// **************************************************************************
// startRide
// **************************************************************************

void ProviderNextbike::startRide(QString unlockId, QGeoCoordinate coordinate,
                                 SupplementaryCallback<QString> callback)
{
    if (!isLoggedIn())
        return;

    qDebug() << "ProviderNextbike::startRide rideId " << unlockId;

    QUrl url(UNLOCK_URL);
    QUrlQuery query;
    query.addQueryItem("apikey", ScooterPrivate::getApiKey("nextbike"));
    query.addQueryItem("loginkey", loginKey);
    query.addQueryItem("show_errors", "true");
    url.setQuery(query.query());

    network::ReqBody body;
    body["bike"] = unlockId;

    net->postJson<
      network::
        ReqCallback>(url, body, defaultHeaders, [this, callback](int err, int code, QByteArray body) {
        qDebug() << "ProviderNextbike::startRide callback";
        qDebug() << "HTTP Code " << code << " Body: " << body;

        if (err != QNetworkReply::NoError || code != 200 || body.isEmpty()) {
            log(Severity::Error,
                "Start ride response: " + QString::number(code) + " (" + body + ")");

            QString errStr = (QString(C::gettext("Starting ride failed")) + " (%1/%2/%3)")
                               .arg(err)
                               .arg(code)
                               .arg(body.isEmpty());
            callback(errStr, "", "");
            return;
        }

        log(Severity::Info, "Start ride response: " + QString::number(code) + " Body: " + body);

        auto doc = QJsonDocument::fromJson(body);
        QJsonObject parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

        if (parsed.isEmpty() || (!parsed.contains("rental") && !parsed.contains("error"))) {
            qDebug() << "Rental 2";
            callback(
              C::gettext(
                "Unlocking failed. Response did not contain ride information. Please re-check ride with official bird app. Issues in response:\n"),
              "", "");
            return;
        }

        if (parsed.contains("error")) {
            QString errStr;
            auto errorObj = parsed["error"].toObject();

            if (errorObj.contains("error_code") && errorObj.contains("message")) {
                auto code = errorObj["error_code"];
                auto message = errorObj["message"];

                errStr = (QString(C::gettext("Starting ride failed")) + " (%1/%2)")
                           .arg(code.toString())
                           .arg(message.toString());
            } else {
                errStr = (QString(C::gettext("Starting ride failed")) + " (Unknown error)");
            }

            callback(errStr, "", "");
            return;
        }

        auto rental = parsed["rental"].toObject();

        if (!rental.contains("id") || !rental["id"].isDouble() || !rental.contains("bike")) {
            callback(
              C::gettext(
                "Problems while unlocking. Please re-check ride with official NextBike app."),
              "", "");
            return;
        }

        callback("", QString::number(rental["id"].toInt()), rental["bike"].toString());
    });
}

// **************************************************************************
// stopRide
// **************************************************************************

void ProviderNextbike::stopRide(QString rideId, QGeoCoordinate coordinate,
                                SupplementaryCallback<QString> callback)
{
    // currently not implemented / not used.

    // stopping a ride is done by physically locking a bike.
}

// **************************************************************************
// checkActiveRide
// **************************************************************************

void ProviderNextbike::checkActiveRide(QString rideId, SupplementaryCallback<QString> callback)
{
    if (!isLoggedIn())
        return;

    QUrl url(RIDE_DETAILS_URL);
    QUrlQuery query;
    query.addQueryItem("apikey", ScooterPrivate::getApiKey("nextbike"));
    query.addQueryItem("loginkey", loginKey);
    query.addQueryItem("show_errors", "true");
    url.setQuery(query.query());

    network::ReqBody body;
    body["rental"] = rideId.toInt();

    net->postJson<network::ReqCallback>(
      url, body, defaultHeaders, [this, callback](int err, int code, QByteArray body) {
          if (err != QNetworkReply::NoError || code != 200 || body.isEmpty()) {
              log(Severity::Error,
                  "Check ride response: " + QString::number(code) + " (" + body + ")");

              QString errStr = (QString(C::gettext("Checking ride failed")) + " (%1/%2/%3)")
                                 .arg(err)
                                 .arg(code)
                                 .arg(body.isEmpty());
              callback(errStr, "", "");
              return;
          }

          log(Severity::Info,
              "Check active ride response: " + QString::number(code) + " Body: " + body);

          if (body.isEmpty())
              return;

          auto doc = QJsonDocument::fromJson(body);
          auto parsed = (!doc.isNull() && doc.isObject()) ? doc.object() : QJsonObject();

          if (parsed.isEmpty() || !parsed.contains("rental") || !parsed["rental"].isObject()) {
              callback(C::gettext("Failed to check active ride (missing information in response)"),
                       "", "");
              return;
          }

          auto rental = parsed["rental"].toObject();

          if (!rental.contains("start_place_lat") || !rental.contains("start_place_lng")
              || !rental.contains("end_place_lng") || !rental.contains("end_place_lat")
              || !rental.contains("start_time") || !rental.contains("end_time")
              || !rental.contains("end_place_name") || !rental.contains("price")
              || !rental.contains("price_service")) {
              callback(C::gettext("Failed to check active ride (missing information in rental)"),
                       "", "");
              return;
          }

          auto endTime = rental["end_time"].toInt();
          auto endLat = rental["end_place_lat"].toDouble();
          auto endLon = rental["end_place_lng"].toDouble();
          auto endPlace = rental["end_place_name"].toString();

          if (endTime == 0 || endLat == 0.0 || endLon == 0.0) {
              return callback("", "", "");
          }

          auto priceCt = (rental["price"].toInt() + rental["price_service"].toInt());
          auto durationS = rental["end_time"].toInt() - rental["start_time"].toInt();
          QGeoCoordinate posStart(rental["start_place_lat"].toDouble(),
                                  rental["start_place_lng"].toDouble());
          QGeoCoordinate posEnd(rental["end_place_lat"].toDouble(),
                                rental["end_place_lng"].toDouble());

          QJsonObject infoObject;
          infoObject["provider"] = "nextbike";
          infoObject["completedAt"] = rental["end_time"];
          infoObject["distance"]
            = QJsonValue(QString::number(posStart.distanceTo(posEnd) / 1000, 'f', 1) + " km");
          infoObject["duration"]
            = QJsonValue(QTime::fromMSecsSinceStartOfDay(durationS * 1000).toString("hh:mm:ss"));
          infoObject["cost"] = priceCt;
          infoObject["currency"] = currency;
          infoObject["locale"] = QLocale::system().name().split("_").first();

          if (endPlace.startsWith("BIKE ")) {
              infoObject["hint"] = C::gettext(
                "Bike has been parked in a flex zone, which leads to an increased service fee");
          }

          callback("", QJsonDocument(infoObject).toJson(QJsonDocument::Compact), "");
      });
}

// **************************************************************************
// saveLogin
// **************************************************************************

void ProviderNextbike::saveLogin(QString account, QString loginKey)
{
    QSettings settings;
    settings.setValue("nextbike/account", account);
    settings.setValue("nextbike/loginKey", loginKey);
}

// **************************************************************************
// clearLogin
// **************************************************************************

void ProviderNextbike::clearLogin()
{
    QSettings settings;
    settings.remove("nextbike/account");
    settings.remove("nextbike/loginKey");

    account.clear();
    loginKey.clear();
}
} // namespace scooter
