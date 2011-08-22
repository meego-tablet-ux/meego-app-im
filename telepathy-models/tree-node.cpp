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

#include <TelepathyQt4Yell/Models/TreeNode>

#include "TelepathyQt4Yell/Models/_gen/tree-node.moc.hpp"

namespace Tpy
{

struct TELEPATHY_QT4_YELL_MODELS_NO_EXPORT TreeNode::Private
{
    Private() :
        mParent(0)
    {
    }

    ~Private()
    {
        qDeleteAll(mChildren);
        mChildren.clear();
    }

    QList<TreeNode *> mChildren;
    TreeNode *mParent;
};

/**
 * \class TreeNode
 * \ingroup models
 * \headerfile TelepathyQt4Yell/tree-node.h <TelepathyQt4Yell/TreeNode>
 *
 * \brief A tree of model items
 *
 * This class is not meant to be used directly, rather subclassed by your own model classes.
 * Each tree node can have a parent and children nodes, so you can use it for hierarchical models,
 * and the items can be of different types, as long as each type subclasses TreeNode.
 */

/**
  * Construct a TreeNode object
  */
TreeNode::TreeNode()
    : mPriv(new Private())
{
}

TreeNode::~TreeNode()
{
    delete mPriv;
}

/**
  * Returns the item located at index
  */
TreeNode *TreeNode::childAt(int index) const
{
    return mPriv->mChildren[index];
}

/**
  * Add a child item to the tree
  */
void TreeNode::addChild(TreeNode *node)
{
    // takes ownership of node
    mPriv->mChildren.append(node);
    node->mPriv->mParent = this;

    // set the parent QObject so that the node doesn't get deleted if used
    // from QML/QtScript
    node->setParent(this);

    // chain changed and removed signals
    connect(node,
            SIGNAL(changed(Tpy::TreeNode*)),
            SIGNAL(changed(Tpy::TreeNode*)));
    connect(node,
            SIGNAL(childrenAdded(Tpy::TreeNode*,QList<Tpy::TreeNode*>)),
            SIGNAL(childrenAdded(Tpy::TreeNode*,QList<Tpy::TreeNode*>)));
    connect(node,
            SIGNAL(childrenRemoved(Tpy::TreeNode*,int,int)),
            SIGNAL(childrenRemoved(Tpy::TreeNode*,int,int)));
}

/**
  * Returns the row index of the given node
  */
int TreeNode::indexOf(TreeNode *node) const {
    return mPriv->mChildren.indexOf(node);
}

/**
  * Returns the number of children of this tree node
  */
int TreeNode::size() const {
    return mPriv->mChildren.size();
}

/**
  * Returns the parent node of this tree node
  */
TreeNode *TreeNode::parent() const
{
    return mPriv->mParent;
}

/**
  * This is not used. It returns an empty QVariant
  */
QVariant TreeNode::data(int role) const
{
    return QVariant();
}

/**
  * This is not used. It always returns false
  */
bool TreeNode::setData(int role, const QVariant &value)
{
    return false;
}

/**
  * Remove this node from the tree. The node's signals are disconnected,
  * its parent is called to remove it, and deleleLater is called.
  */
void TreeNode::remove()
{
    if (mPriv->mParent) {
        disconnect(this,
                   SIGNAL(changed(Tpy::TreeNode*)),
                   mPriv->mParent,
                   SIGNAL(changed(Tpy::TreeNode*)));
        disconnect(this,
                   SIGNAL(childrenAdded(Tpy::TreeNode*,QList<Tpy::TreeNode*>)),
                   mPriv->mParent,
                   SIGNAL(childrenAdded(Tpy::TreeNode*,QList<Tpy::TreeNode*>)));
        disconnect(this,
                   SIGNAL(childrenRemoved(Tpy::TreeNode*,int,int)),
                   mPriv->mParent,
                   SIGNAL(childrenRemoved(Tpy::TreeNode*,int,int)));

        mPriv->mParent->mPriv->mChildren.removeOne(this);
    }
    deleteLater();
}

}
