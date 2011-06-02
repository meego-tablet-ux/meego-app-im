#include "imdbus.h"
#include "imdbusadaptor_p.h"
#include <QDebug>

IMDBus::IMDBus(QObject *parent) :
    QObject(parent)
{
    mAdaptor = new IMDBusAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/com/meego/app/im", this);
    QDBusConnection::sessionBus().registerService("com.meego.app.im");

}


void IMDBus::showChat(const QString &accountId, const QString &contactId)
{
    emit chatOpenRequested(accountId, contactId);
}

void IMDBus::acceptCall(const QString &accountId, const QString &contactId)
{
    emit callAccepted(accountId, contactId);
}