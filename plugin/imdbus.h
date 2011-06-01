#ifndef IMDBUS_H
#define IMDBUS_H

#include <QObject>

class IMDBusAdaptor;

class IMDBus : public QObject
{
    Q_OBJECT
public:
    explicit IMDBus(QObject *parent = 0);

signals:
    void callAccepted(const QString &accountId, const QString &contactId);
    void chatOpenRequested(const QString &accountId, const QString &contactId);

public slots:
    void showChat(const QString &accountId, const QString &contactId);
    void acceptCall(const QString &accountId, const QString &contactId);

private:
    IMDBusAdaptor *mAdaptor;
};

#endif // IMDBUS_H
