/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_tree_node_h_HEADER_GUARD_
#define _Telepathy_Models_tree_node_h_HEADER_GUARD_

#include "global.h"

#include <QObject>
#include <QVariant>

class TELEPATHY_MODELS_EXPORT TreeNode : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TreeNode)

public:
    TreeNode();

    virtual ~TreeNode();

    TreeNode *childAt(int index) const;

    void addChild(TreeNode *node);

    int indexOf(TreeNode *node) const;

    int size() const;

    TreeNode *parent() const;

    virtual QVariant data(int role) const;
    virtual bool setData(int role, const QVariant &value);
    virtual void remove();

Q_SIGNALS:
    void changed(TreeNode *);
    void childrenAdded(TreeNode *parent, const QList<TreeNode *> &nodes);
    void childrenRemoved(TreeNode *parent, int first, int last);

private:
    struct Private;
    friend struct Private;
    Private *mPriv;
};

#endif // _Telepathy_Models_tree_node_h_HEADER_GUARD_
