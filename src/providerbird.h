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

#ifndef PROVIDERBIRD_H
#define PROVIDERBIRD_H

#include <QObject>
#include <QJsonObject>

#include "network.h"
#include "provider.h"

class QGeoPositionInfoSource;

// **************************************************************************
// namespace scooter
// **************************************************************************

namespace scooter
{
   class ProviderBird : public Provider
   {
         Q_OBJECT

      public:
         explicit ProviderBird(network::Network* net, QGeoPositionInfoSource* position, QObject* parent = nullptr);

         bool isLoggedIn() override { return !accessToken.isEmpty(); };

         void init() override;
         void login(QString loginId, QString password = "") override;
         void logout() override;
         void confirmLogin(QString confirmation) override;
         void refreshLogin() override;
         void getProfile() override;
         void reloadConfig(QGeoCoordinate) override;

         void reloadScooters(QGeoCoordinate, int radius, ResultCallback<ScooterMap>) override;
         void reloadAreas(QGeoCoordinate coordinates, int radius) override;

         void ringScooter(QString id, QGeoCoordinate);
         void scanScooter(QString qrCode, QGeoCoordinate, ResultCallback<Scooter>) override;
         void startRide(QString unlockId, QGeoCoordinate, ResultCallback<QString>) override;
         void stopRide(QString rideId, QGeoCoordinate, SupplementaryCallback<QString>) override;

      private:
         void _reloadConfig();
         void loadActiveRide(QGeoCoordinate coordinates);
         void addVoucherCode(QString code);
         void saveLogin(const QJsonObject& params);
         void clearLogin();

         QString deviceID;
         QString accessToken;
         QString refreshToken;

         network::ReqHeaders defaultHeaders;
   };
}
#endif // PROVIDERBIRD_H
