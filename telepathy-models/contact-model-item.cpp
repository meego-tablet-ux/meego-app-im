/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "contact-model-item.h"

//#include "TelepathyQt4Yell/Models/_gen/contact-model-item.moc.hpp"

#include "accounts-model.h"
#include <TelepathyQt4Yell/CallChannel>
#include <TelepathyQt4Yell/ContactCapabilities>
#include <TelepathyQt4Yell/RequestableChannelClassSpec>

#include <TelepathyQt4/AvatarData>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/ContactCapabilities>

#include <QImage>

struct TELEPATHY_MODELS_NO_EXPORT ContactModelItem::Private
{
    Private(const Tp::ContactPtr &contact)
        : mContact(contact),
          mContactCaps(contact->capabilities())
    {
    }

    Tp::ContactPtr mContact;
    Tpy::ContactCapabilities mContactCaps;
};

/**
 * \class ContactModelItem
 * \ingroup models
 * \headerfile TelepathyQt4Yell/contact-model-item.h <TelepathyQt4Yell/ContactModelItem>
 *
 * \brief This is a model item that represents a Telepathy Contact
 *
 */

/**
  * Construct a ContactModelItem object
  * \param contact a Tp::ContactPtr object that references the Telepathy contact
  */
ContactModelItem::ContactModelItem(const Tp::ContactPtr &contact)
    : mPriv(new Private(contact))
{

    connect(contact.data(),
            SIGNAL(aliasChanged(QString)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(avatarTokenChanged(QString)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(avatarDataChanged(Tp::AvatarData)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(presenceChanged(Tp::Presence)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(capabilitiesChanged(Tp::ContactCapabilities)),
            SLOT(onCapabilitiesChanged()));
    connect(contact.data(),
            SIGNAL(locationUpdated(Tp::LocationInfo)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(infoFieldsChanged(Tp::Contact::InfoFields)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(publishStateChanged(Tp::Contact::PresenceState,QString)),
            SLOT(onChanged()));
    connect(contact.data(),
            SIGNAL(blockStatusChanged(bool)),
            SLOT(onChanged()));
}

ContactModelItem::~ContactModelItem()
{
    delete mPriv;
}

QVariant ContactModelItem::data(int role) const
{
    switch (role)
    {
        case AccountsModel::ItemRole:
            return QVariant::fromValue(
                const_cast<QObject *>(
                    static_cast<const QObject *>(this)));
        case AccountsModel::IdRole:
            return mPriv->mContact->id();
        case Qt::DisplayRole:
        case AccountsModel::AliasRole:
            return mPriv->mContact->alias();
        case AccountsModel::PresenceStatusRole:
            return mPriv->mContact->presence().status();
        case AccountsModel::PresenceTypeRole:
            return mPriv->mContact->presence().type();
        case AccountsModel::PresenceMessageRole:
            return mPriv->mContact->presence().statusMessage();
        case AccountsModel::SubscriptionStateRole:
            return mPriv->mContact->subscriptionState();
        case AccountsModel::PublishStateRole:
            return mPriv->mContact->publishState();
        case AccountsModel::BlockedRole:
            return mPriv->mContact->isBlocked();
        case AccountsModel::GroupsRole:
            return mPriv->mContact->groups();
        case AccountsModel::AvatarRole:
            return mPriv->mContact->avatarData().fileName;
        case Qt::DecorationRole:
            return QImage(mPriv->mContact->avatarData().fileName);
        case AccountsModel::TextChatCapabilityRole:
            return mPriv->mContact->capabilities().textChats();
        case AccountsModel::StreamedMediaCallCapabilityRole:
            return mPriv->mContact->capabilities().streamedMediaCalls();
        case AccountsModel::StreamedMediaAudioCallCapabilityRole:
            return mPriv->mContact->capabilities().streamedMediaAudioCalls();
        case AccountsModel::StreamedMediaVideoCallCapabilityRole:
            return mPriv->mContact->capabilities().streamedMediaVideoCalls();
        case AccountsModel::StreamedMediaVideoCallWithAudioCapabilityRole:
            return mPriv->mContact->capabilities().streamedMediaVideoCallsWithAudio();
        case AccountsModel::StreamedMediaUpgradeCallCapabilityRole:
            return mPriv->mContact->capabilities().upgradingStreamedMediaCalls();
        case AccountsModel::MediaCallCapabilityRole:
            return mPriv->mContactCaps.mediaCalls();
        case AccountsModel::AudioCallCapabilityRole:
            return mPriv->mContactCaps.audioCalls();
        case AccountsModel::VideoCallCapabilityRole:
            return mPriv->mContactCaps.videoCalls();
        case AccountsModel::VideoCallWithAudioCapabilityRole:
            return mPriv->mContactCaps.videoCallsWithAudio();
        case AccountsModel::UpgradeCallCapabilityRole:
            return mPriv->mContactCaps.upgradingCalls();
        case AccountsModel::FileTransferCapabilityRole:
            return mPriv->mContactCaps.fileTransfers();
        default:
            break;
    }

    return QVariant();
}

/**
  * Set data for a role of the object.
  * Currently, the only supported roles are PublishStateRole and SubscriptionStateRole,
  * which authorize or remove publication and subscription, depending on value.
  * \param role The role that will be set the value
  * \param value The value that will be set
  */
bool ContactModelItem::setData(int role, const QVariant &value)
{
    switch (role) {
        case AccountsModel::PublishStateRole: {
            Tp::Contact::PresenceState state;
            state = (Tp::Contact::PresenceState) value.toInt();
            switch (state) {
                case Tp::Contact::PresenceStateYes:
                    // authorize the contact and request its presence publication
                    mPriv->mContact->authorizePresencePublication();
                    return true;
                case Tp::Contact::PresenceStateNo: {
                    // reject the presence publication and remove the contact
                    mPriv->mContact->removePresencePublication();
                    return true;
                }
                default:
                    return false;
            }
        }
        case AccountsModel::SubscriptionStateRole: {
            Tp::Contact::PresenceState state;
            state = (Tp::Contact::PresenceState) value.toInt();
            switch (state) {
                case Tp::Contact::PresenceStateAsk:
                    // authorize the contact and request its presence publication
                    mPriv->mContact->requestPresenceSubscription();
                    return true;
                case Tp::Contact::PresenceStateNo: {
                    // reject the presence publication and remove the contact
                    mPriv->mContact->removePresenceSubscription();
                    return true;
                }
                default:
                    return false;
            }
        }
        default:
            return false;
    }
}

void ContactModelItem::onChanged()
{
    emit changed(this);
}

/**
  * Returns the contact object of the item
  */
Tp::ContactPtr ContactModelItem::contact() const
{
    return mPriv->mContact;
}

void ContactModelItem::onCapabilitiesChanged()
{
    // TODO: double check this, the original code is the commented one
    //mPriv->mContactCaps.updateRequestableChannelClasses(mPriv->mContact->capabilities().allClassSpecs().bareClasses());
    mPriv->mContactCaps = mPriv->mContact->capabilities();
    emit capabilitiesChanged();
}

/**
  * Remove a contact from the known contacts list
  * \param message text to be sent when removing the contact
  */
void ContactModelItem::remove(const QString &message)
{
    QList<Tp::ContactPtr> contacts;
    contacts << mPriv->mContact;
    mPriv->mContact->manager()->removeContacts(contacts, message);
    onChanged();
}
