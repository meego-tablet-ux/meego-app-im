/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "improtocolsmodel.h"
#include <QDir>
#include <QDebug>
#include <QDeclarativeComponent>
#include <QDeclarativeEngine>

IMProtocolsModel::IMProtocolsModel(QObject *parent) :
    QAbstractListModel(parent),
    mDefaultCustomizer(0)
{
    QHash<int, QByteArray> roles;

    roles[TitleRole] = "title";
    roles[IconRole] = "icon";
    roles[IdRole] = "id";
    roles[ConnectionManagerRole] = "connectionManager";
    roles[ProtocolRole] = "protocol";
    roles[SingleInstanceRole] = "singleInstance";
    setRoleNames(roles);

    QDir dir(modulePath());
    if (!dir.exists() || !dir.isReadable()) {
        return;
    }

    QDeclarativeEngine *mEngine = new QDeclarativeEngine(parent);
    if (!mEngine) {
        qWarning() << "Error, declarative engine could not be created";
    }

    if (mEngine) {
        qDebug() << "adding path " << modulePath();
        //mEngine->setBaseUrl(modulePath());
        mEngine->importPlugin("/usr/share/meego-app-im/Customizer.qml");
        QDeclarativeComponent component(mEngine, QUrl::fromLocalFile("Customizer.qml"));
        if (component.isReady()) {
            mDefaultCustomizer = component.create();
            mDefaultCustomizer->setParent(this);
        }
    }

    foreach (const QString &entry, dir.entryList(QStringList() << "*.desktop")) {
        MDesktopEntry *desktopEntry = new MDesktopEntry(dir.absoluteFilePath(entry));
        if (desktopEntry->value("Desktop Entry", "Type") != "IMProtocol") {
            delete desktopEntry;
            continue;
        }

        QString protocolId = desktopEntry->value("MTI", "Id");
        qDebug() << "Protocol found " << protocolId;
        mProtocolList.append(desktopEntry);
        mProtocolMap[protocolId] = desktopEntry;

        if (mEngine) {
            QString customizer = desktopEntry->value("MTI", "Customizer");
            if (!customizer.isEmpty()) {
                QDeclarativeComponent component(mEngine, QUrl::fromLocalFile(customizer));
                qDebug() << "errorString=" << component.errorString();
                if (component.isReady()) {
                    qDebug() << "Customizer found " << customizer;
                    QObject *customizer = component.create();
                    customizer->setParent(this);
                    mCustomizerMap[protocolId] = customizer;
                }
            }
        }
    }

}

IMProtocolsModel::~IMProtocolsModel()
{
    foreach (QObject *customizer, mCustomizerMap) {
        delete customizer;
    }

    if (mEngine) {
        delete mEngine;
        mEngine = 0;
    }
}

int IMProtocolsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mProtocolList.count();
}

QVariant IMProtocolsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mProtocolList.count()) {
        return QVariant();
    }

    switch (role) {
    case TitleRole:
        return mProtocolList[index.row()]->name();
    case IconRole:
        return mProtocolList[index.row()]->value("MTI", "Icon");
    case IdRole:
        return mProtocolList[index.row()]->value("MTI", "Id");
    case ConnectionManagerRole:
        return mProtocolList[index.row()]->value("MTI", "ConnectionManager");
    case ProtocolRole:
        return mProtocolList[index.row()]->value("MTI", "Protocol");
    case SingleInstanceRole:
        if (mProtocolList[index.row()]->contains("MTI", "SingleInstance")) {
            return (mProtocolList[index.row()]->value("MTI", "SingleInstance") == "true");
        }
        return false;
    }

    return QVariant();
}

QString IMProtocolsModel::iconForId(const QString &id) const
{
    if (!mProtocolMap.contains(id)) {
        return QString::null;
    }

    return mProtocolMap[id]->value("MTI", "Icon");
}

QString IMProtocolsModel::contentForId(const QString &id) const
{
    if (!mProtocolMap.contains(id)) {
        return QString::null;
    }

    return mProtocolMap[id]->value("MTI", "Content");
}

QString IMProtocolsModel::titleForId(const QString &id) const
{
    if (!mProtocolMap.contains(id)) {
        return QString::null;
    }

    return mProtocolMap[id]->name();
}

bool IMProtocolsModel::isSingleInstance(const QString &id) const
{
    if (!mProtocolMap.contains(id) ||
        !mProtocolMap[id]->contains("MTI", "SingleInstance")) {
        return false;
    }

   return (mProtocolMap[id]->value("MTI", "SingleInstance") == "true");
}

QMap<QString, QString> IMProtocolsModel::protocolNames() const
{
    QMap<QString,QString> names;
    foreach (MDesktopEntry *entry, mProtocolList) {
        names[entry->value("MTI", "Id")] = entry->name();
    }

    return names;
}

QString IMProtocolsModel::modulePath() const
{
    return QString::fromLatin1("/usr/share/meego-app-im/protocols/");
}

QObject *IMProtocolsModel::customizerForId(const QString &id) const
{
    if (!mCustomizerMap.contains(id)) {
        return mDefaultCustomizer;
    }

    return mCustomizerMap[id];
}
