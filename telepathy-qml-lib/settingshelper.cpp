/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "settingshelper.h"

SettingsHelper *SettingsHelper::mSelf = 0;

SettingsHelper::SettingsHelper(QObject *parent) :
    QObject(parent), mSettings("MeeGo", "MeeGoIM")
{
}

SettingsHelper *SettingsHelper::self()
{
    if (!mSelf) {
        mSelf = new SettingsHelper();
    }

    return mSelf;
}

bool SettingsHelper::showOfflineContacts()
{
    mSettings.sync();
    return mSettings.value("ShowOfflineContacts", true).toBool();
}

void SettingsHelper::setShowOfflineContacts(bool show)
{
    mSettings.setValue("ShowOfflineContacts", show);
    mSettings.sync();
    emit showOfflineContactsChanged();
}

bool SettingsHelper::enableAudioAlerts()
{
    mSettings.sync();
    return mSettings.value("EnableAudioAlerts", true).toBool();
}

void SettingsHelper::setEnableAudioAlerts(bool enable)
{
    mSettings.setValue("EnableAudioAlerts", enable);
    mSettings.sync();
    emit enableAudioAlertsChanged();
}

bool SettingsHelper::enableNotifications()
{
    mSettings.sync();
    return mSettings.value("EnableNotifications", true).toBool();
}

void SettingsHelper::setEnableNotifications(bool enable)
{
    mSettings.setValue("EnableNotifications", enable);
    mSettings.sync();
    emit enableNotificationsChanged();
}

bool SettingsHelper::enableVibrate()
{
    mSettings.sync();
    return mSettings.value("EnableVibrate", true).toBool();
}

void SettingsHelper::setEnableVibrate(bool enable)
{
    mSettings.setValue("EnableVibrate", enable);
    mSettings.sync();
    emit enableVibrateChanged();
}

QVariant SettingsHelper::value(const QString &prop, const QVariant &defaultValue)
{
    mSettings.sync();
    return mSettings.value(prop, defaultValue);
}

void SettingsHelper::setValue(const QString &prop, const QVariant &value)
{
    mSettings.setValue(prop, value);
    mSettings.sync();
    emit valueUpdated(prop);
}
