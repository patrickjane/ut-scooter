// **************************************************************************
// class ProviderStub
// empty implementaton of scooter provider
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

#ifndef PROVIDERSTUB_H
#define PROVIDERSTUB_H

#include <QGeoPositionInfoSource>
#include <QJsonObject>
#include <QObject>

#include "network.h"
#include "provider.h"

// **************************************************************************
// namespace scooter
// **************************************************************************

namespace scooter {
class ProviderStub : public Provider {
    Q_OBJECT

public:
    explicit ProviderStub(network::Network* net, QGeoPositionInfoSource* position,
                          QObject* parent = nullptr)
      : Provider(net, position, parent)
    {
    }

    bool isLoggedIn() override
    {
        return false;
    };

    void init() override
    {
        fReady = true;
        emit ready();
    }
    void login(QString, QString) override
    {
    }
    void logout() override
    {
    }
    void confirmLogin(QString) override
    {
    }
    void refreshLogin() override
    {
    }
    void getProfile() override
    {
    }
    void reloadConfig(QGeoCoordinate) override
    {
    }

    void reloadScooters(QGeoCoordinate, int) override
    {
    }
    void reloadAreas(QGeoCoordinate, int) override
    {
    }

    void ringScooter(QString, QGeoCoordinate) override
    {
    }
    void scanScooter(QString, QGeoCoordinate, ResultCallback<Scooter>) override
    {
    }
    void startRide(QString, QGeoCoordinate, SupplementaryCallback<QString>) override
    {
    }
    void stopRide(QString, QGeoCoordinate, SupplementaryCallback<QString>) override
    {
    }

private:
    void _reloadConfig();
    void saveLogin(const QJsonObject& params);
    void clearLogin();

    QString deviceID;
    QString accessToken;
    QString refreshToken;

    network::ReqHeaders defaultHeaders;
};
} // namespace scooter
#endif // PROVIDERSTUB_H
