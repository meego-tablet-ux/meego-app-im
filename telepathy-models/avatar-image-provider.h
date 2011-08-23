/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_avatar_image_provider_h_HEADER_GUARD_
#define _Telepathy_Models_avatar_image_provider_h_HEADER_GUARD_

#include "global.h"

#include <TelepathyQt4/Types>

#include <QDeclarativeImageProvider>
#include <QDeclarativeEngine>

class TELEPATHY_MODELS_EXPORT AvatarImageProvider : public QDeclarativeImageProvider
{
    Q_DISABLE_COPY(AvatarImageProvider)

public:
    AvatarImageProvider(const Tp::AccountManagerPtr &am);
    virtual ~AvatarImageProvider();

    static QString urlFor(const Tp::AccountPtr &account);
    static void registerProvider(QDeclarativeEngine *engine, const Tp::AccountManagerPtr &am);

    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_avatar_image_provider_h_HEADER_GUARD_
