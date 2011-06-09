#include "resourcesetmanager.h"

#define APPLICATION_CLASS_NO_POLICY "nopolicy"

using namespace ResourcePolicy;

ResourceSetManager::ResourceSetManager(QObject *parent) :
    QObject(parent),
    mResourceSet(0),
    mWaitingForResources(false),
    mAcquired(false)
{
    qDebug() << "ResourceSetManager::ResourceSetManager";
}

ResourceSetManager::~ResourceSetManager()
{
    qDebug() << "ResourceSetManager::~ResourceSetManager";

    if (mResourceSet) {
        mResourceSet->release();
        delete mResourceSet;
        mResourceSet = 0;
    }
}

QString ResourceSetManager::applicationClass() const
{
    return mApplicationClass;
}

void ResourceSetManager::setApplicationClass(const QString &appClass)
{
    qDebug() << "ResourceSetManager::setApplicationClass: " << appClass;

    if (mApplicationClass == appClass) {
        return;
    }

    if (mResourceSet) {
        mResourceSet->release();
        delete mResourceSet;
        mResourceSet = 0;
    }

    mApplicationClass = appClass;

    if (!mApplicationClass.isEmpty() && mApplicationClass != APPLICATION_CLASS_NO_POLICY) {
        mResourceSet = new ResourcePolicy::ResourceSet(mApplicationClass, this);
        if (mResourceSet) {
            mResourceSet->setAutoRelease();
            mResourceSet->setAlwaysReply();
            mNeedInitAndConnect = true;
            mNeedUpdate = false;

            connect(mResourceSet,
                    SIGNAL(resourcesGranted(QList<ResourcePolicy::ResourceType>)),
                    SLOT(onResourceSetCallGranted()));
            connect(mResourceSet,
                    SIGNAL(lostResources()),
                    SLOT(onResourceSetCallLost()));
            connect(mResourceSet,
                    SIGNAL(resourcesDenied()),
                    SLOT(onResourceSetCallDenied()));
            connect(mResourceSet,
                    SIGNAL(errorCallback(quint32,const char*)),
                    SLOT(onResourceSetCallError(quint32,const char *)));
        }
    }

    emit applicationClassChanged();
}

bool ResourceSetManager::waitingForResources() const
{
    return mWaitingForResources;
}

bool ResourceSetManager::acquired() const
{
    return mAcquired;
}

void ResourceSetManager::addAudioResource(const QString &audioGroup)
{
    qDebug() << "ResourceSetManager::addAudioGroup: " << audioGroup;

    if (mResourceSet && !audioGroup.isEmpty()) {
        ResourcePolicy::AudioResource *audioResource = new ResourcePolicy::AudioResource(audioGroup);
        if (audioResource) {
            audioResource->setProcessID(QCoreApplication::applicationPid());
            audioResource->setStreamTag("media.name", "*");
            audioResource->setOptional(false);
            mResourceSet->addResourceObject(audioResource);
            mNeedUpdate = true;
        }
    }
}

void ResourceSetManager::addResource(ResourceType type)
{
    qDebug() << "ResourceSetManager::addResource: " << type;

    if (mResourceSet) {
        mResourceSet->addResource(ResourcePolicy::ResourceType(type));
        mNeedUpdate = true;
    }
}

void ResourceSetManager::deleteResource(ResourceType type)
{
    qDebug() << "ResourceSetManager::deleteResource: " << type;

    if (mResourceSet) {
        mResourceSet->deleteResource(ResourcePolicy::ResourceType(type));
        mNeedUpdate = true;
    }
}

void ResourceSetManager::acquire()
{
    qDebug() << "ResourceSetManager::acquire";

    if (mApplicationClass == APPLICATION_CLASS_NO_POLICY) {
        emit beginUsage();
        return;
    }

    if (!mResourceSet) {
        emit endUsage();
        return;
    }

    if (mNeedInitAndConnect) {
        mResourceSet->initAndConnect();
        mNeedInitAndConnect = false;
        mNeedUpdate = false;
    }

    if (mNeedUpdate) {
        mResourceSet->update();
        mNeedUpdate = false;
    }

    mWaitingForResources = true;
    mResourceSet->acquire();
}

void ResourceSetManager::release()
{
    if (!mResourceSet) {
        emit endUsage();
        return;
    }

    mWaitingForResources = false;
    mAcquired = false;
    mResourceSet->release();
}

void ResourceSetManager::onResourceSetCallGranted()
{
    qDebug() << "ResourceSetManager::onResourceSetCallGranted:";

    mWaitingForResources = false;
    mAcquired = true;
    emit beginUsage();
}

void ResourceSetManager::onResourceSetCallLost()
{
    qDebug() << "ResourceSetManager::onResourceSetCallLost";

    mWaitingForResources = false;
    emit endUsage();
}

void ResourceSetManager::onResourceSetCallDenied()
{
    qDebug() << "ResourceSetManager::onResourceSetCallDenied";

    mWaitingForResources = false;
    emit endUsage();
}

void ResourceSetManager::onResourceSetCallError(quint32 error,const char *message)
{
    qDebug() << "ResourceSetManager::onResourceSetCallError: error=" << error << " msg=" << message;

    mWaitingForResources = false;
    emit endUsage();
}
