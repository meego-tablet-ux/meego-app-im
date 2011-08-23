/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
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
