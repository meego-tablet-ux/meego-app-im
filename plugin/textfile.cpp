#include "textfile.h"
#include <QFile>

TextFile::TextFile(QObject *parent) :
    QObject(parent),
    mValid(false)
{
}

QString TextFile::text() const
{
    return mText;
}

QString TextFile::source() const
{
    return mSource;
}

void TextFile::setSource(const QString &file)
{
    mValid = false;
    mSource = file;

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }

    mText = f.readAll();
    mValid = !mText.isEmpty();
    emit sourceChanged();
    emit textChanged();
    emit validChanged();
}
