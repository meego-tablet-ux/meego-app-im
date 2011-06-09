/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef RESOURCESETMANAGER_H
#define RESOURCESETMANAGER_H

#include <QObject>
#include <policy/resource-set.h>

class ResourceSetManager : public QObject
{
    Q_OBJECT

    Q_ENUMS(ResourceType)
    Q_PROPERTY(QString applicationClass READ applicationClass WRITE setApplicationClass NOTIFY applicationClassChanged);

public:

    enum ResourceType {
        AudioPlaybackType = ResourcePolicy::AudioPlaybackType,  ///< For audio playback
        VideoPlaybackType = ResourcePolicy::VideoPlaybackType,  ///< For video playback
        AudioRecorderType = ResourcePolicy::AudioRecorderType,  ///< For audio recording (using of the microphone)
        VideoRecorderType = ResourcePolicy::VideoRecorderType,  ///< For video recording (using the camera)
        VibraType = ResourcePolicy::VibraType,                  ///< For Vibra
        LedsType = ResourcePolicy::LedsType,                    ///< For LEDs
        BacklightType = ResourcePolicy::BacklightType,          ///< For the backlight (of the display)
        SystemButtonType = ResourcePolicy::SystemButtonType,    ///< For the system (power) button
        LockButtonType = ResourcePolicy::LockButtonType,        ///< For the lock button
        ScaleButtonType = ResourcePolicy::ScaleButtonType,      ///< The scale (zoom) button
        SnapButtonType = ResourcePolicy::SnapButtonType,        ///< Use this if you are a camera application
        LensCoverType = ResourcePolicy::LensCoverType,
        HeadsetButtonsType = ResourcePolicy::HeadsetButtonsType,///< Use this to reserve the headset buttons
    };

    ResourceSetManager(QObject *parent = 0);
    ~ResourceSetManager();

    QString applicationClass() const;
    void setApplicationClass(const QString &appClass);

    Q_INVOKABLE bool waitingForResources() const;
    Q_INVOKABLE bool acquired() const;

signals:
    void applicationClassChanged();
    void beginUsage();
    void endUsage();

public slots:
    void addAudioResource(const QString &audioGroup);
    void addResource(ResourceType type);
    void deleteResource(ResourceType type);
    void acquire();
    void release();

private slots:
    void onResourceSetCallGranted();
    void onResourceSetCallLost();
    void onResourceSetCallDenied();
    void onResourceSetCallError(quint32,const char *);

private:
    QString mApplicationClass;
    ResourcePolicy::ResourceSet *mResourceSet;
    bool mWaitingForResources;
    bool mAcquired;
    bool mNeedInitAndConnect;
    bool mNeedUpdate;
};

#endif // RESOURCESETMANAGER_H
