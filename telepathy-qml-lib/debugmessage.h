/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef DEBUGMESSAGE_H
#define DEBUGMESSAGE_H

#include <QDBusArgument>
#include <QDateTime>
#include <QList>
#include <QString>

enum DebugLevel {
    DebugLevelError = 0,
    DebugLevelCritical = 1,
    DebugLevelWarning = 2,
    DebugLevelMessage = 3,
    DebugLevelInfo = 4,
    DebugLevelDebug = 5
};

struct DebugMessage
{
public:
    DebugMessage(const double &time, const QString &domain, DebugLevel level, const QString &message);
    DebugMessage();

    static void registerMetaType();

    double time;
    QString domain;
    DebugLevel level;
    QString message;
};

QDBusArgument &operator << (QDBusArgument &arg, const DebugMessage &message);
const QDBusArgument &operator >> (const QDBusArgument &arg, DebugMessage &message);

typedef QList<DebugMessage> DebugMessageList;

Q_DECLARE_METATYPE(DebugMessage);
Q_DECLARE_METATYPE(DebugMessageList);

#endif // DEBUGMESSAGE_H
