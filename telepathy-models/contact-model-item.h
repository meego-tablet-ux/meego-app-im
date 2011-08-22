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

#ifndef _Telepathy_Models_contact_model_item_h_HEADER_GUARD_
#define _Telepathy_Models_contact_model_item_h_HEADER_GUARD_

#include "tree-node.h"

#include <TelepathyQt4/Types>

class TELEPATHY_MODELS_EXPORT ContactModelItem : public TreeNode
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactModelItem)

public:
    ContactModelItem(const Tp::ContactPtr &contact);
    virtual ~ContactModelItem();

    Q_INVOKABLE virtual QVariant data(int role) const;
    Q_INVOKABLE virtual bool setData(int role, const QVariant &value);
    Q_INVOKABLE void remove(const QString &message = QString());

    Tp::ContactPtr contact() const;

Q_SIGNALS:
    void capabilitiesChanged();

public Q_SLOTS:
    void onChanged();
    void onCapabilitiesChanged();

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_contact_model_item_h_HEADER_GUARD_
