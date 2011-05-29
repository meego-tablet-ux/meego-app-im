#ifndef TEXTFILE_H
#define TEXTFILE_H

#include <QObject>

class TextFile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)

public:
    explicit TextFile(QObject *parent = 0);

    QString text() const;

    QString source() const;
    void setSource(const QString &file);

    bool valid() const;

signals:
    void textChanged();
    void sourceChanged();
    void validChanged();

private:
    QString mText;
    QString mSource;
    bool mValid;

};

#endif // TEXTFILE_H
