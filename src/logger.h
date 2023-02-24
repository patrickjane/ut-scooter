// **************************************************************************
// class Logger
// Logging functionality
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

#pragma once

#include <QObject>

// **************************************************************************
// namespace scooter
// **************************************************************************

namespace scooter {
enum class Severity : int { Info = 0, Warning, Error };

class Logger : public QObject {
    Q_OBJECT

public:
    explicit Logger(QObject* parent = nullptr) : QObject(parent)
    {
    }

    void log(Severity severity, QString message)
    {
        emit logMessage(severity, message);
    }

    void log(Severity severity, QString provider, QString message)
    {
        QString sv;

        switch (severity) {
        case Severity::Info:
            sv = "INF";
            break;
        case Severity::Warning:
            sv = "WRN";
            break;
        case Severity::Error:
            sv = "ERR";
            break;
        }

        emit logMessage2(sv, provider, message);
    }

signals:
    void logMessage(Severity severity, QString message);
    void logMessage2(QString severity, QString provider, QString message);
};
}; // namespace scooter
