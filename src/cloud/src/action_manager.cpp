#include <QSet>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include <rcl_cloud_action.h>
#include <rbl_error.h>
#include <rbl_logger.h>

#include "action_manager.h"

ActionManager::ActionManager(const ActionManagerSettings &settings, QObject *parent)
    : QObject{parent}
    , settings{settings}
{
    this->statistics.setName(this->settings.getName());

    if (QFile::exists(this->settings.getFileName()))
    {
        this->readFile();
    }

    QSet<QString> loadedActions;
    for (auto iter = this->actions.cbegin();iter!=this->actions.cend();++iter)
    {
        loadedActions.insert(iter->getName());
    }

    QMap<QString,QString> actionMap = RCloudAction::getActionMap();
    for (auto iter = actionMap.cbegin(); iter != actionMap.cend(); ++iter)
    {
        const QString &actionName = iter.key();

        if (loadedActions.contains(actionName))
        {
            continue;
        }

        RCloudActionInfo actionInfo;
        actionInfo.setName(actionName);

        RAccessOwner accessOwner;
        accessOwner.setUser(RUserInfo::rootUser);
        if (actionName == RCloudAction::Action::FileUpdateAccessOwner::key ||
            actionName == RCloudAction::Action::Stop::key ||
            actionName == RCloudAction::Action::Statistics::key ||
            actionName == RCloudAction::Action::Process::key ||
            actionName == RCloudAction::Action::UserAdd::key ||
            actionName == RCloudAction::Action::UserUpdate::key ||
            actionName == RCloudAction::Action::UserRemove::key ||
            actionName == RCloudAction::Action::GroupAdd::key ||
            actionName == RCloudAction::Action::GroupRemove::key ||
            actionName == RCloudAction::Action::ActionUpdateAccessOwner::key ||
            actionName == RCloudAction::Action::ActionUpdateAccessMode::key ||
            actionName == RCloudAction::Action::ProcessUpdateAccessOwner::key ||
            actionName == RCloudAction::Action::ProcessUpdateAccessMode::key)
        {
            accessOwner.setGroup(RUserInfo::rootGroup);
        }
        else
        {
            accessOwner.setGroup(RUserInfo::userGroup);
        }

        RAccessMode accessMode;
        accessMode.setUserModeMask(RAccessMode::Mode::Execute);
        accessMode.setGroupModeMask(RAccessMode::Mode::Execute);
        accessMode.setOtherModeMask(RAccessMode::Mode::None);

        if (actionName == RCloudAction::Action::Test::key ||
            actionName == RCloudAction::Action::ListFiles::key ||
            actionName == RCloudAction::Action::FileInfo::key ||
            actionName == RCloudAction::Action::FileDownload::key ||
            actionName == RCloudAction::Action::UserRegister::key ||
            actionName == RCloudAction::Action::Process::key ||
            actionName == RCloudAction::Action::SubmitReport::key)
        {
            accessMode.setOtherModeMask(RAccessMode::Mode::Execute);
        }

        RAccessRights accessRights;
        accessRights.setOwner(accessOwner);
        accessRights.setMode(accessMode);

        actionInfo.setAccessRights(accessRights);

        this->actions.append(actionInfo);
    }

    this->writeFile();
}

bool ActionManager::containsAction(const QString &name) const
{
    foreach (const RCloudActionInfo &a, this->actions)
    {
        if (a.getName() == name)
        {
            return true;
        }
    }
    return false;
}

RCloudActionInfo ActionManager::findAction(const QString &name) const
{
    foreach (const RCloudActionInfo &a, this->actions)
    {
        if (a.getName() == name)
        {
            return a;
        }
    }
    return RCloudActionInfo();
}

RCloudActionInfo ActionManager::updateActionAccessRights(const QString &name, const RAccessRights &accessRights)
{
    if (!accessRights.isValid())
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Invalid access rights \"%s\".",accessRights.toString().toUtf8().constData());
    }
    for (qsizetype i=0;i<this->actions.size();i++)
    {
        if (this->actions[i].getName() == name)
        {
            RLogger::info("[%s] Updating action \"%s\" access-rights: \"%s\".\n",
                          this->settings.getName().toUtf8().constData(),
                          name.toUtf8().constData(),
                          accessRights.toString().toUtf8().constData());
            this->actions[i].setAccessRights(accessRights);
            return this->actions[i];
        }
    }
    throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Action name \"%s\" does not exist.",name.toUtf8().constData());
}

const QList<RCloudActionInfo> &ActionManager::getActions() const
{
    return this->actions;
}

void ActionManager::readFile()
{
    RLogger::info("[%s] Reading actions file \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  this->settings.getFileName().toUtf8().constData());

    QFile inFile(this->settings.getFileName());

    if (!inFile.exists())
    {
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"Actions file \"%s\" does not exist.",this->settings.getFileName().toUtf8().constData());
    }

    if(!inFile.open(QIODevice::ReadOnly))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open actions file \"%s\" for reading. %s.",
                     inFile.fileName().toUtf8().constData(),
                     inFile.errorString().toUtf8().constData());
    }

    QByteArray byteArray = inFile.readAll();
    RLogger::info("[%s] Successfuly read \"%ld\" bytes from \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  byteArray.size(),inFile.fileName().toUtf8().constData());

    this->fromJson(QJsonDocument::fromJson(byteArray).object());

    inFile.close();
}

void ActionManager::writeFile() const
{
    RLogger::info("[%s] Writing actions file \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  this->settings.getFileName().toUtf8().constData());

    QFile outFile(this->settings.getFileName());

    if(!outFile.open(QIODevice::WriteOnly))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open actions file \"%s\" for writing. %s.",
                     outFile.fileName().toUtf8().constData(),
                     outFile.errorString().toUtf8().constData());
    }

    qint64 bytesOut = outFile.write(QJsonDocument(this->toJson()).toJson());

    RLogger::info("[%s] Successfuly wrote \"%ld\" bytes to \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  bytesOut,
                  outFile.fileName().toUtf8().constData());

    outFile.close();
}

bool ActionManager::authorizeUser(const RUserInfo &userInfo, const QString &name) const
{
    return this->findAction(name).getAccessRights().isUserAuthorized(userInfo,RAccessMode::Mode::Execute);
}

QJsonObject ActionManager::getStatisticsJson() const
{
    RLogger::debug("[%s] Producting statistics\n",this->settings.getName().toUtf8().constData());
    ServiceStatistics snapshotStatistics(this->statistics);
    snapshotStatistics.recordCounter("size",this->actions.size());
    return snapshotStatistics.toJson();
}

void ActionManager::fromJson(const QJsonObject &json)
{
    if (const QJsonValue &v = json["actions"]; v.isArray())
    {
        const QJsonArray &actionsArray = v.toArray();
        this->actions.clear();
        this->actions.reserve(actionsArray.size());
        for (const QJsonValue &action : actionsArray)
        {
            this->actions.append(RCloudActionInfo::fromJson(action.toObject()));
        }
    }
}

QJsonObject ActionManager::toJson() const
{
    QJsonObject json;

    // Actions
    QJsonArray actionsArray;
    for (const RCloudActionInfo &actionInfo : this->actions)
    {
        actionsArray.append(actionInfo.toJson());
    }
    json["actions"] = actionsArray;

    return json;
}
