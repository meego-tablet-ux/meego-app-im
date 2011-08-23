/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "avatar-image-provider.h"

#include <TelepathyQt4/Account>
#include <TelepathyQt4/AccountManager>

struct TELEPATHY_MODELS_NO_EXPORT AvatarImageProvider::Private
{
    Private(const Tp::AccountManagerPtr &am)
        : mAM(am)
    {
    }

    Tp::AccountManagerPtr mAM;
};

AvatarImageProvider::AvatarImageProvider(const Tp::AccountManagerPtr &am)
    : QDeclarativeImageProvider(Image),
      mPriv(new Private(am))
{
}

AvatarImageProvider::~AvatarImageProvider()
{
    delete mPriv;
}

QString AvatarImageProvider::urlFor(const Tp::AccountPtr &account)
{
    return QString::fromLatin1("image://avatars/") + account->uniqueIdentifier();
}

void AvatarImageProvider::registerProvider(QDeclarativeEngine *engine, const Tp::AccountManagerPtr &am)
{
    engine->addImageProvider(QString::fromLatin1("avatars"), new AvatarImageProvider(am));
}

QImage AvatarImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);
    QString path = QString::fromLatin1(TELEPATHY_ACCOUNT_OBJECT_PATH_BASE "/") + id;
    Tp::AccountPtr account = mPriv->mAM->accountForPath(path);
    QImage image;
    if (!account.isNull() && account->isValid()) {
        image.loadFromData(account->avatar().avatarData);
        if (size) {
            *size = image.size();
        }
    }
    return image;
}
