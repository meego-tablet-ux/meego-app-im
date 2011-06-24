#ifndef IMDBUS_H
#define IMDBUS_H

#include <QObject>

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

public slots:
    void showChat(const QString &accountId, const QString &contactId);
    void reportMissedCalls(const QString &accountId, const QStringList &contacts);
    void dumpLogs();
    bool userBusyCalling();

private:
    IMDBusAdaptor *mAdaptor;
    IMAccountsModel *mAccountsModel;
};

#endif // IMDBUS_H
