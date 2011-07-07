#include "imdbus.h"
#include "imdbusadaptor_p.h"
#include "imaccountsmodel.h"
#include "../telepathy-qml-lib/telepathymanager.h"
#include <QDebug>


IMDBus::IMDBus(QObject *parent) :
    QObject(parent), mAccountsModel(0)
{
    mAdaptor = new IMDBusAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/com/meego/app/im", this);
    QDBusConnection::sessionBus().registerService("com.meego.app.im");
}

QObject *IMDBus::accountsModel() const
{
    return mAccountsModel;
}

void IMDBus::setAccountsModel(QObject *model)
{
    mAccountsModel = qobject_cast<IMAccountsModel*>(model);
}


bool IMDBus::userBusyCalling()
{
    if (!mAccountsModel) {
        return false;
    }

    return mAccountsModel->userBusyCalling();
}

void IMDBus::showChat(const QString &accountId, const QString &contactId)
{
    emit chatOpenRequested(accountId, contactId);
}

void IMDBus::showGroupChat(const QString &accountId, const QString &groupChatId)
{
    emit groupChatOpenRequested(accountId, groupChatId);
}

void IMDBus::reportMissedAudioCalls(const QString &accountId, const QStringList &contacts, const QStringList &times)
{
    // if we dont have an instance of Accounts Model we cant report missed calls
    if (!mAccountsModel) {
        return;
    }

    for (int i = 0; i < contacts.count(); ++ i) {
        mAccountsModel->reportMissedAudioCall(accountId, contacts[i], QDateTime::fromString(times[i]));
    }
}

void IMDBus::reportMissedVideoCalls(const QString &accountId, const QStringList &contacts, const QStringList &times)
{
    // if we dont have an instance of Accounts Model we cant report missed calls
    if (!mAccountsModel) {
        return;
    }

    for (int i = 0; i < contacts.count(); ++ i) {
        mAccountsModel->reportMissedVideoCall(accountId, contacts[i], QDateTime::fromString(times[i]));
    }
}

void IMDBus::dumpLogs()
{
    TelepathyManager::instance()->dumpLogs();
}
