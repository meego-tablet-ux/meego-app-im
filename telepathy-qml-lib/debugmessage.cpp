/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "debugmessage.h"
#include <QtDBus>

DebugMessage::DebugMessage() :
    level(DebugLevelDebug)
{
}

DebugMessage::DebugMessage(const double &aTime, const QString &aDomain, DebugLevel aLevel, const QString &aMessage) :
    time(aTime),
    domain(aDomain),
    level(aLevel),
    message(aMessage)
{
}

void DebugMessage::registerMetaType()
{
    qRegisterMetaType<DebugMessage>("DebugMessage");
    qDBusRegisterMetaType<DebugMessage>();
    qRegisterMetaType<DebugMessageList>("DebugMessageList");
    qDBusRegisterMetaType<DebugMessageList>();
}

QDBusArgument &operator << (QDBusArgument &arg, const DebugMessage &message)
{
    arg.beginStructure();
    arg << message.time << message.domain << uint(message.level) << message.message;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator >> (const QDBusArgument &arg, DebugMessage &message)
{
    arg.beginStructure();
    uint tmpLevel;
    arg >> message.time >> message.domain >> tmpLevel >> message.message;
    arg.endStructure();
    message.level = DebugLevel(tmpLevel);
    return arg;
}
