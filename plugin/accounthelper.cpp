/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "accounthelper.h"
#include "imaccountsmodel.h"
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingAccount>
#include <TelepathyQt4/PendingStringList>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/ProtocolParameter>

#include <TelepathyQt4Yell/Models/AccountsModelItem>
#include <QUrl>
#include <QImage>

AccountHelper::AccountHelper(QObject *parent) :
    QObject(parent),
    mSaslAuth(false),
    mAccountsModel(0),
    mAllowTextChatFrom(0),
    mAllowCallFrom(0),
    mAllowOutsideCallFrom(0),
    mShowMyAvatarTo(0),
    mShowMyWebStatus(0),
    mShowIHaveVideoTo(0),
    mConnectsAutomatically(true)
{
    mAccountManager = Tp::AccountManager::create();
    connect(mAccountManager->becomeReady(), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

QString AccountHelper::connectionManager() const
{
    return mConnectionManager;
}

void AccountHelper::setConnectionManager(const QString &value)
{
    mConnectionManager = value;
    emit connectionManagerChanged();
}

QString AccountHelper::protocol() const
{
    return mProtocol;
}

void AccountHelper::setProtocol(const QString &value)
{
    mProtocol = value;
    emit protocolChanged();
}

QString AccountHelper::displayName() const
{
    return mDisplayName;
}

void AccountHelper::setDisplayName(const QString &value)
{
    mDisplayName = value;
    emit displayNameChanged();
}

QString AccountHelper::password() const
{
    return mPassword;
}

void AccountHelper::setPassword(const QString &value)
{
    mPassword = value;
    emit passwordChanged();
}

QString AccountHelper::icon() const
{
    return mIcon;
}

void AccountHelper:: setIcon(const QString &value)
{
    mIcon = value;
}

QString AccountHelper::avatar() const
{
    return mAvatar;
}

void AccountHelper::setAvatar(const QString &value)
{
    if (mAccount.isNull()) {
        return;
    }

    QString localFile = QUrl(value).path();
    mAvatar = localFile;
    qDebug() << "Setting avatar from file" << localFile;

    QImage img(localFile);
    img = img.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QByteArray ba;
    QBuffer buf(&ba);

    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");

    Tp::Avatar accountAvatar;
    accountAvatar.avatarData = ba;
    accountAvatar.MIMEType = "image/png";
    mAccount->setAvatar(accountAvatar);
    emit avatarChanged();
}

bool AccountHelper::saslAuth() const
{
    return mSaslAuth;
}

void AccountHelper::setSaslAuth(bool value)
{
    mSaslAuth = value;
    emit saslAuthChanged();
}

QObject *AccountHelper::accountsModel() const
{
    return mAccountsModel;
}

void AccountHelper::setAccountsModel(QObject *value)
{
    mAccountsModel = qobject_cast<IMAccountsModel*>(value);
    emit accountsModelChanged();
}

void AccountHelper::setAccountParameter(const QString &property, const QVariant &value)
{
    mParameters[property] = value;
}

QVariant AccountHelper::accountParameter(const QString &property) const
{
    if (!mParameters.contains(property))
        return QVariant();
    return mParameters[property];
}

void AccountHelper::unsetAccountParameter(const QString &property)
{
    mUnsetParameters.append(property);
    if (mParameters.contains(property)) {
        mParameters.remove(property);
    }
}

void AccountHelper::createAccount()
{
    mParameters["account"] = mDisplayName;

    if (!mSaslAuth) {
        mParameters["password"] = mPassword;
    }

    if (mAccount.isNull()) {
        Tp::PendingAccount *pa = mAccountManager->createAccount(mConnectionManager,
                                                                 mProtocol,
                                                                 mDisplayName,
                                                                 mParameters);
        connect(pa, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onAccountCreated(Tp::PendingOperation*)));
    }
    else {
        if (mSaslAuth && mAccountsModel) {
            mAccountsModel->setAccountPassword(mAccount->uniqueIdentifier(),
                                               mPassword);
        }
        Tp::ProtocolInfo info = mAccount->protocolInfo();
        Tp::ProtocolParameterList protocolParameters = info.parameters();

        // remove all parameters that are equal to the default value
        // and set the correct type
        foreach (Tp::ProtocolParameter param, protocolParameters) {
            if (mParameters.contains(param.name())) {
                if (mParameters[param.name()] == param.defaultValue()) {
                    mParameters.remove(param.name());
                    mUnsetParameters.append(param.name());
                } else {
                    // set the correct type
                    if (mParameters[param.name()].type() != param.type()) {
                        QString name = param.name();
                        QVariant value = mParameters[name];
                        switch (param.type()) {
                            case QVariant::Int:
                                mParameters[param.name()] = value.toInt();
                                break;
                            case QVariant::UInt:
                                mParameters[param.name()] = value.toUInt();
                                break;
                            default:
                                break;
                        }
                        // TODO: check if there is any other type that needs conversion
                    }
                }
            }
        }

        QVariantMap::iterator it = mParameters.begin();

        /*while (it != mParameters.end()) {
            qDebug() << "Going to set parameter " << it.key() << " to " << it.value();
            ++it;
        }*/

        // set account to connect automatically
        mAccount->setConnectsAutomatically(mConnectsAutomatically);
        mAccount->setDisplayName(mDisplayName);

        Tp::PendingStringList *psl = mAccount->updateParameters(mParameters, mUnsetParameters);
        connect(psl, SIGNAL(finished(Tp::PendingOperation*)),
                this, SLOT(onParametersUpdated(Tp::PendingOperation*)));
        updatePrivacySettings();
    }
}

void AccountHelper::setAccount(QObject *object)
{
    Tpy::AccountsModelItem *accountItem = qobject_cast<Tpy::AccountsModelItem*>(object);
    mAccount = accountItem->account();
    emit onlineChanged();

    mParameters = mAccount->parameters();
    mPassword = mParameters["password"].toString();

    mConnectsAutomatically = mAccount->connectsAutomatically();

    // load the default parameter values
    Tp::ProtocolInfo info = mAccount->protocolInfo();
    Tp::ProtocolParameterList protocolParameters = info.parameters();

    foreach (Tp::ProtocolParameter param, protocolParameters) {
        if (!mParameters.contains(param.name())) {
            QVariant value = param.defaultValue();
            if (value.isNull()) {
                continue;
            }
            mParameters[param.name()] = param.defaultValue();
        }
    }

    // get privacy settings
    qDebug() << "AccountHelper::setAccount: loading privacy settings";
    onPrivacySettingsLoaded(mAccount->uniqueIdentifier());
    connect(mAccountsModel, SIGNAL(privacyPropertiesLoaded(QString)),
            SLOT(onPrivacySettingsLoaded(QString)));
    connect(mAccount.data(), SIGNAL(connectionStatusChanged(Tp::ConnectionStatus)),
            SIGNAL(onlineChanged()));
}

void AccountHelper::onPrivacySettingsLoaded(const QString &accountId)
{
    if (mAccount && mAccountsModel
            && mAccount->uniqueIdentifier() == accountId) {
        Tpy::AccountsModelItem *accountItem = qobject_cast<Tpy::AccountsModelItem*>(mAccountsModel->accountItemForId(
                    mAccount->uniqueIdentifier()));
        if (accountItem) {
            setAllowTextChatFrom(mAccountsModel->privacySetting( mAccount->uniqueIdentifier(), IMAccountsModel::AllowTextChannelsFromRole).toUInt());
            setAllowCallFrom(mAccountsModel->privacySetting( mAccount->uniqueIdentifier(), IMAccountsModel::AllowCallChannelsFromRole).toUInt());
            setAllowOutsideCallFrom(mAccountsModel->privacySetting( mAccount->uniqueIdentifier(), IMAccountsModel::AllowOutsideCallsFromRole).toUInt());
            setShowMyAvatarTo(mAccountsModel->privacySetting( mAccount->uniqueIdentifier(), IMAccountsModel::ShowMyAvatarRole).toUInt());
            setShowMyWebStatus(mAccountsModel->privacySetting( mAccount->uniqueIdentifier(), IMAccountsModel::ShowMyWebStatusRole).toUInt());
            setShowIHaveVideoTo(mAccountsModel->privacySetting( mAccount->uniqueIdentifier(), IMAccountsModel::ShowIHaveVideoToRole).toUInt());
        }
    }
}

void AccountHelper::removeAccount()
{
    if (mAccount.isNull()) {
        return;
    }

    mAccount->remove();
}

QString AccountHelper::accountId() const
{
    if (mAccount.isNull()) {
        return QString();
    }
    return mAccount->uniqueIdentifier();
}

void AccountHelper::onAccountManagerReady(Tp::PendingOperation *op)
{
    Q_UNUSED(op);

    // TODO: check what we need to do here
}

void AccountHelper::onAccountCreated(Tp::PendingOperation *op)
{
    if (op->isError()) {
        // TODO: check how to notify errors
        return;
    }

    Tp::PendingAccount *pendingAccount = qobject_cast<Tp::PendingAccount*>(op);
    if (!pendingAccount) {
        // TODO: notify error
        return;
    }

    mAccount = pendingAccount->account();

    if (mSaslAuth && mAccountsModel) {
        mAccountsModel->setAccountPassword(mAccount->uniqueIdentifier(),
                                           mPassword);
    }

    // set account to connect automatically
    mAccount->setConnectsAutomatically(mConnectsAutomatically);
    qDebug("account set to automatic");

    // get the account online
    connect(mAccount->setEnabled(true), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountEnabled(Tp::PendingOperation*)));
}

void AccountHelper::onAccountEnabled(Tp::PendingOperation *op)
{
    if (op->isError()) {
        // TODO: notify errors and get back to the setup screen
        return;
    }

    connect(mAccount->setIconName(mIcon), SIGNAL(finished(Tp::PendingOperation*)),
            this, SLOT(onAccountIconSet(Tp::PendingOperation*)));
}

void AccountHelper::onAccountIconSet(Tp::PendingOperation *op)
{
    if (op->isError()) {
        // TODO: notify errors and get back to the setup screen
        // do not return, we can continue executing.
    }

    Tp::SimplePresence presence;
    presence.type = Tp::ConnectionPresenceTypeAvailable;
    presence.status = "online";
    presence.statusMessage = "";

    mAccount->setRequestedPresence(presence);
    emit accountSetupFinished();
}

void AccountHelper::onParametersUpdated(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qDebug() << "Error:" << op->errorMessage();
        return;
    }
    Tp::PendingStringList *list = qobject_cast<Tp::PendingStringList*>(op);
    qDebug() << "Parameters on relogin:" << list->result();

    if (list->result().count()) {
        mAccount->reconnect();
    }

    emit accountSetupFinished();
}

void AccountHelper::updatePrivacySettings()
{
    if (mAccount && mAccountsModel) {
        mAccountsModel->setPrivacySetting(mAccount->uniqueIdentifier(), IMAccountsModel::AllowTextChannelsFromRole, mAllowTextChatFrom);
        mAccountsModel->setPrivacySetting(mAccount->uniqueIdentifier(), IMAccountsModel::AllowCallChannelsFromRole, mAllowCallFrom);
        mAccountsModel->setPrivacySetting(mAccount->uniqueIdentifier(), IMAccountsModel::AllowOutsideCallsFromRole, mAllowOutsideCallFrom);
        mAccountsModel->setPrivacySetting(mAccount->uniqueIdentifier(), IMAccountsModel::ShowMyAvatarRole, mShowMyAvatarTo);
        mAccountsModel->setPrivacySetting(mAccount->uniqueIdentifier(), IMAccountsModel::ShowMyWebStatusRole, mShowMyWebStatus);
        mAccountsModel->setPrivacySetting(mAccount->uniqueIdentifier(), IMAccountsModel::ShowIHaveVideoToRole, mShowIHaveVideoTo);
    }
}

uint AccountHelper::allowTextChatFrom() const
{
    return mAllowTextChatFrom;
}

uint AccountHelper::allowCallFrom() const
{
    return mAllowCallFrom;
}

uint AccountHelper::allowOutsideCallFrom() const
{
    return mAllowOutsideCallFrom;
}

uint AccountHelper::showMyAvatarTo() const
{
    return mShowMyAvatarTo;
}

uint AccountHelper::showMyWebStatus() const
{
    return mShowMyWebStatus;
}

uint AccountHelper::showIHaveVideoTo() const
{
    return mShowIHaveVideoTo;
}

void AccountHelper::setAllowTextChatFrom(const uint &value)
{
    mAllowTextChatFrom = value;
    emit allowTextChatFromChanged();
}

void AccountHelper::setAllowCallFrom(const uint &value)
{
    mAllowCallFrom = value;
    emit allowTextChatFromChanged();
}

void AccountHelper::setAllowOutsideCallFrom(const uint &value)
{
    mAllowOutsideCallFrom = value;
    emit allowOutsideCallFromChanged();
}

void AccountHelper::setShowMyAvatarTo(const uint &value)
{
    mShowMyAvatarTo = value;
    emit showMyAvatarToChanged();
}

void AccountHelper::setShowMyWebStatus(const uint &value)
{
    mShowMyWebStatus = value;
    emit showMyWebStatusChanged();
}

void AccountHelper::setShowIHaveVideoTo(const uint &value)
{
    mShowIHaveVideoTo = value;
    emit showIHaveVideoToChanged();
}

bool AccountHelper::isOnline() const
{
    if (mAccount) {
        Tp::ConnectionPtr connection = mAccount->connection();
        if (!connection.isNull() && connection->isValid()) {
            if (connection->status() == Tp::ConnectionStatusConnected) {
                return true;
            }
        }
    }
    return false;
}

bool AccountHelper::connectsAutomatically() const
{
    return mConnectsAutomatically;
}

void AccountHelper::setConnectsAutomatically(bool value)
{
    mConnectsAutomatically = value;
    emit connectsAutomaticallyChanged();
}
