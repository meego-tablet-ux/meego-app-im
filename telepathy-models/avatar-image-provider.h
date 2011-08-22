/*
 * This file is part of TelepathyQt4Yell Models
 *
 * Copyright (C) 2010 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
