#ifndef IMDBUS_H
#define IMDBUS_H

#include <QObject>
#include <QVariantList>

class IMDBusAdaptor;
class IMAccountsModel;

class IMDBus : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *model READ accountsModel WRITE setAccountsModel)
public:
    explicit IMDBus(QObject *parent = 0);

    QObject *accountsModel() const;
    void setAccountsModel(QObject *model);

signals:
    void chatOpenRequested(const QString &accountId, const QString &contactId);
    void groupChatOpenRequested(const QString &accountId, const QString &groupChatId);

public slots:
    void showChat(const QString &accountId, const QString &contactId);
    void showGroupChat(const QString &accountId, const QString &groupChatId);
    void reportMissedAudioCalls(const QString &accountId, const QStringList &contacts, const QStringList &times);
    void reportMissedVideoCalls(const QString &accountId, const QStringList &contacts, const QStringList &times);
    void dumpLogs();
    bool userBusyCalling();

private:
    IMDBusAdaptor *mAdaptor;
    IMAccountsModel *mAccountsModel;
};

#endif // IMDBUS_H
