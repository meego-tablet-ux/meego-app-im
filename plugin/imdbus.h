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
    void chatOpenRequested(const QString &accountId, const QString &contactId);

public slots:
    void showChat(const QString &accountId, const QString &contactId);
    void dumpLogs();

private:
    IMDBusAdaptor *mAdaptor;
};

#endif // IMDBUS_H
