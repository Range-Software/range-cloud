#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include <rbl_error.h>
#include <rbl_logger.h>

#include "user_manager.h"

UserManager::AuthTokenValidator::AuthTokenValidator(UserManager *pUserManager)
    : RAuthTokenValidator{pUserManager}
    , pUserManager{pUserManager}
{

}

bool UserManager::AuthTokenValidator::validate(const QString &resourceName, const QString &token)
{
    RAuthToken authToken = this->pUserManager->findToken(resourceName,token);

    if (authToken.isNull())
    {
        return false;
    }

    bool isValid = false;
    isValid = (authToken.getValidityDate() > QDateTime::currentSecsSinceEpoch());

    try
    {
        this->pUserManager->removeToken(authToken.getId());
    }
    catch (const RError &rError)
    {
        RLogger::error("[UserTokenAuthenticator] Failed to remove used authentication token \"%s\"\n");
    }

    return isValid;
}

UserManager::UserManager(const UserManagerSettings &settings, QObject *parent)
    : QObject{parent}
    , settings{settings}
{
    this->statistics.setName(this->settings.getName());

    if (QFile::exists(this->settings.getFileName()))
    {
        this->readFile();
    }
    else
    {
        RGroupInfo rootGroup;
        rootGroup.setName(RUserInfo::rootGroup);
        this->addGroup(rootGroup);

        RGroupInfo guestGroup;
        guestGroup.setName(RUserInfo::guestGroup);
        this->addGroup(guestGroup);

        RGroupInfo usersGroup;
        usersGroup.setName(RUserInfo::userGroup);
        this->addGroup(usersGroup);

        RUserInfo rootUser;
        rootUser.setName(RUserInfo::rootUser);
        rootUser.getGroupNames().append(rootGroup.getName());
        this->addUser(rootUser);

        RUserInfo guestUser;
        guestUser.setName(RUserInfo::guestUser);
        guestUser.getGroupNames().append(guestGroup.getName());
        this->addUser(guestUser);

        this->writeFile();
    }

    this->authTokenValidator = new AuthTokenValidator(this);

    QObject::connect(this,&UserManager::changed,this,&UserManager::onChanged);
    QObject::connect(this,&UserManager::groupRemoved,this,&UserManager::onGroupRemoved);
}

bool UserManager::containsUser(const QString &name) const
{
    foreach (const RUserInfo &u, this->users)
    {
        if (u.getName() == name)
        {
            return true;
        }
    }
    return false;
}

RUserInfo UserManager::findUser(const QString &name) const
{
    foreach (const RUserInfo &u, this->users)
    {
        if (u.getName() == name)
        {
            return u;
        }
    }
    return RUserInfo();
}

bool UserManager::containsGroup(const QString &name) const
{
    foreach (const RGroupInfo &g, this->groups)
    {
        if (g.getName() == name)
        {
            return true;
        }
    }
    return false;
}

RGroupInfo UserManager::findGroup(const QString &name) const
{
    foreach (const RGroupInfo &g, this->groups)
    {
        if (g.getName() == name)
        {
            return g;
        }
    }
    return RGroupInfo();
}

bool UserManager::containsToken(const QString &resourceName, const QString &content) const
{
    foreach (const RAuthToken &t, this->tokens)
    {
        if (t.getResourceName() == resourceName && t.getContent() == content)
        {
            return true;
        }
    }
    return false;
}

bool UserManager::containsToken(const QUuid &id) const
{
    foreach (const RAuthToken &t, this->tokens)
    {
        if (t.getId() == id)
        {
            return true;
        }
    }
    return false;
}

RAuthToken UserManager::findToken(const QString &resourceName, const QString &content) const
{
    foreach (const RAuthToken &t, this->tokens)
    {
        if (t.getResourceName() == resourceName && t.getContent() == content)
        {
            return t;
        }
    }
    return RAuthToken();
}

RAuthToken UserManager::findToken(const QUuid &id) const
{
    foreach (const RAuthToken &t, this->tokens)
    {
        if (t.getId() == id)
        {
            return t;
        }
    }
    return RAuthToken();
}

void UserManager::addUser(const RUserInfo &user)
{
    RLogger::info("[%s] Add user \"%s\" \'%s\'.\n",
                  this->settings.getName().toUtf8().constData(),
                  user.getName().toUtf8().constData(),
                  QJsonDocument(user.toJson()).toJson(QJsonDocument::Compact).constData());
    if (!RUserInfo::isNameValid(user.getName()))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"User is not valid.");
    }
    foreach (const RUserInfo &u, this->users)
    {
        if (u.getName() == user.getName())
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,"User with given ID or name already exists.");
        }
    }

    for (const QString &groupName : user.getGroupNames())
    {
        if (!this->containsGroup(groupName))
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,"User group \"%s\" does not exist.",groupName.toUtf8().constData());
        }
    }

    this->users.append(user);

    emit this->changed();
    emit this->userAdded(this->users.last());
}

void UserManager::setUser(const QString &userName, const RUserInfo &user)
{
    RLogger::info("[%s] Set user \"%s\" \'%s\'.\n",
                  this->settings.getName().toUtf8().constData(),
                  userName.toUtf8().constData(),
                  QJsonDocument(user.toJson()).toJson(QJsonDocument::Compact).constData());
    if (!RUserInfo::isNameValid(user.getName()))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"User is not valid.");
    }

    for (const QString &groupName : user.getGroupNames())
    {
        if (!this->containsGroup(groupName))
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,"User group \"%s\" does not exist.",groupName.toUtf8().constData());
        }
    }

    QMutableListIterator<RUserInfo> i(this->users);
    while (i.hasNext())
    {
        RUserInfo u = i.next();
        if (u.getName() == userName)
        {
            i.setValue(user);
            emit this->changed();
            emit this->userChanged(i.value());
            return;
        }
    }

    throw RError(RError::Type::InvalidInput,R_ERROR_REF,"User with given ID does not exist.");
}

void UserManager::removeUser(const QString &name)
{
    RLogger::info("[%s] Remove user \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  name.toUtf8().constData());
    QMutableListIterator<RUserInfo> i(this->users);
    while (i.hasNext())
    {
        RUserInfo u = i.next();
        if (u.getName() == name)
        {
            i.remove();
            emit this->changed();
            emit this->userRemoved(u);
            return;
        }
    }

    throw RError(RError::Type::InvalidInput,R_ERROR_REF,"User with given ID does not exist.");
}

void UserManager::addGroup(const RGroupInfo &group)
{
    RLogger::info("[%s] Add group \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  group.getName().toUtf8().constData());
    if (!RGroupInfo::isNameValid(group.getName()))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Group is not valid.");
    }
    foreach (const RGroupInfo &g, this->groups)
    {
        if (g.getName() == group.getName())
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Group with given ID or name already exists.");
        }
    }

    this->groups.append(group);

    emit this->changed();
    emit this->groupAdded(this->groups.last());
}

void UserManager::setGroup(const RGroupInfo &group)
{
    RLogger::info("[%s] Set group \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  group.getName().toUtf8().constData());
    if (!RGroupInfo::isNameValid(group.getName()))
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Group is not valid.");
    }

    QMutableListIterator<RGroupInfo> i(this->groups);
    while (i.hasNext())
    {
        RGroupInfo g = i.next();
        if (g.getName() == group.getName())
        {
            i.setValue(group);
            emit this->changed();
            emit this->groupChanged(i.value());
            return;
        }
    }

    throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Group with given ID does not exist.");
}

void UserManager::removeGroup(const QString &name)
{
    RLogger::info("[%s] Remove group \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  name.toUtf8().constData());
    QMutableListIterator<RGroupInfo> i(this->groups);
    while (i.hasNext())
    {
        RGroupInfo g = i.next();
        if (g.getName() == name) {
            i.remove();
            emit this->changed();
            emit this->groupRemoved(g);
            return;
        }
    }

    throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Group with given ID does not exist.");
}

void UserManager::addToken(const RAuthToken &token)
{
    RLogger::info("[%s] Add token id = \"%s\", resource = \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  token.getId().toString(QUuid::WithoutBraces).toUtf8().constData(),
                  token.getResourceName().toUtf8().constData());
    if (token.isNull())
    {
        throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Token is not valid.");
    }
    foreach (const RAuthToken &t, this->tokens)
    {
        if (t.getId() == token.getId())
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Token with given ID already exists.");
        }
        if (t.getResourceName() == token.getResourceName() && t.getContent() == token.getContent())
        {
            throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Token with given resource name and content already exists.");
        }
    }

    this->tokens.append(token);

    emit this->changed();
    emit this->tokenAdded(this->tokens.last());
}

void UserManager::removeToken(const QUuid &id)
{
    RLogger::info("[%s] Remove token id = \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  id.toString(QUuid::WithoutBraces).toUtf8().constData());
    QMutableListIterator<RAuthToken> i(this->tokens);
    while (i.hasNext())
    {
        RAuthToken t = i.next();
        if (t.getId() == id) {
            i.remove();
            emit this->changed();
            emit this->tokenRemoved(t);
            return;
        }
    }

    throw RError(RError::Type::InvalidInput,R_ERROR_REF,"Token with given ID does not exist.");
}

UserManager::AuthTokenValidator *UserManager::getAuthTokenValidator()
{
    return this->authTokenValidator;
}

void UserManager::readFile()
{
    RLogger::info("[%s] Reading users file \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),
                  this->settings.getFileName().toUtf8().constData());

    QFile inFile(this->settings.getFileName());

    if (!inFile.exists())
    {
        throw RError(RError::Type::InvalidFileName,R_ERROR_REF,"Users file \"%s\" does not exist.",this->settings.getFileName().toUtf8().constData());
    }

    if(!inFile.open(QIODevice::ReadOnly))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open users file \"%s\" for reading. %s.",
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

void UserManager::writeFile() const
{
    RLogger::info("[%s] Writing users file \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),this->settings.getFileName().toUtf8().constData());

    QFile outFile(this->settings.getFileName());

    if(!outFile.open(QIODevice::WriteOnly))
    {
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open users file \"%s\" for writing. %s.",
                     outFile.fileName().toUtf8().constData(),
                     outFile.errorString().toUtf8().constData());
    }

    qint64 bytesOut = outFile.write(QJsonDocument(this->toJson()).toJson());

    RLogger::info("[%s] Successfuly wrote \"%ld\" bytes to \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),bytesOut,outFile.fileName().toUtf8().constData());

    outFile.close();
}

QList<QString> UserManager::getUserNames() const
{
    QList<QString> userNames;

    userNames.reserve(this->users.count());

    for (const RUserInfo &userInfo : this->users)
    {
        userNames.append(userInfo.getName());
    }

    return userNames;
}

QList<QString> UserManager::getGroupNames() const
{
    QList<QString> groupNames;

    groupNames.reserve(this->groups.count());

    for (const RGroupInfo &groupInfo : this->groups)
    {
        groupNames.append(groupInfo.getName());
    }

    return groupNames;
}

bool UserManager::authorizeUserAccess(const RUserInfo &userInfo, const RAccessRights &accessRights, const RAccessMode::Mode &accessMode)
{
    if (accessMode == RAccessMode::None)
    {
        // Verify ownership
        return (userInfo.isUser(RUserInfo::rootUser) || userInfo.getName() == accessRights.getOwner().getUser());
    }
    else
    {
        // Verify access
        return (userInfo.isUser(RUserInfo::rootUser) || userInfo.hasGroup(RUserInfo::rootGroup) || accessRights.isUserAuthorized(userInfo,accessMode));
    }
}

RUserInfo UserManager::createUser(const QString &userName)
{
    RUserInfo userInfo;

    userInfo.setName(userName);
    userInfo.getGroupNames().append(RUserInfo::userGroup);

    return userInfo;
}

RGroupInfo UserManager::createGroup(const QString &groupName)
{
    RGroupInfo groupInfo;

    groupInfo.setName(groupName);

    return groupInfo;
}

QJsonObject UserManager::getStatisticsJson() const
{
    RLogger::debug("[%s] Producting statistics\n",this->settings.getName().toUtf8().constData());
    ServiceStatistics snapshotStatistics(this->statistics);
    snapshotStatistics.recordCounter("users",this->users.size());
    snapshotStatistics.recordCounter("users",this->groups.size());
    return snapshotStatistics.toJson();
}

void UserManager::fromJson(const QJsonObject &json)
{
    if (const QJsonValue &v = json["users"]; v.isArray())
    {
        const QJsonArray &usersArray = v.toArray();
        this->users.clear();
        this->users.reserve(usersArray.size());
        for (const QJsonValue &user : usersArray)
        {
            this->users.append(RUserInfo::fromJson(user.toObject()));
        }
    }

    if (const QJsonValue &v = json["groups"]; v.isArray())
    {
        const QJsonArray &groupsArray = v.toArray();
        this->groups.clear();
        this->groups.reserve(groupsArray.size());
        for (const QJsonValue &group : groupsArray)
        {
            this->groups.append(RGroupInfo::fromJson(group.toObject()));
        }
    }

    if (const QJsonValue &v = json["tokens"]; v.isArray())
    {
        const QJsonArray &tokensArray = v.toArray();
        this->tokens.clear();
        this->tokens.reserve(tokensArray.size());
        for (const QJsonValue &token : tokensArray)
        {
            this->tokens.append(RAuthToken::fromJson(token.toObject()));
        }
    }
}

QJsonObject UserManager::toJson() const
{
    QJsonObject json;

    // Users
    QJsonArray usersArray;
    for (const RUserInfo &user : this->users)
    {
        usersArray.append(user.toJson());
    }
    json["users"] = usersArray;

    // Groups
    QJsonArray groupsArray;
    foreach (const RGroupInfo &group, this->groups)
    {
        groupsArray.append(group.toJson());
    }
    json["groups"] = groupsArray;

    // Tokens
    QJsonArray tokensArray;
    foreach (const RAuthToken &token, this->tokens)
    {
        tokensArray.append(token.toJson());
    }
    json["tokens"] = tokensArray;

    return json;
}

void UserManager::onChanged()
{
    try
    {
        this->writeFile();
    }
    catch (const RError &error)
    {
        RLogger::error("[%s] Failed to write users file \"%s\". %s",
                       this->settings.getName().toUtf8().constData(),
                       this->settings.getFileName().toUtf8().constData(),
                       error.getMessage().toUtf8().constData());
    }
    catch (...)
    {
        RLogger::error("[%s] Failed to write users file \"%s\". %s",
                       this->settings.getName().toUtf8().constData(),
                       this->settings.getFileName().toUtf8().constData(),
                       RError::getTypeMessage(RError::Unknown).toUtf8().constData());
    }
}

void UserManager::onGroupRemoved(RGroupInfo group)
{
    QMutableListIterator<RUserInfo> i(this->users);
    while (i.hasNext())
    {
        RUserInfo u = i.next();
        if (u.getGroupNames().contains(group.getName()))
        {
            if (i.value().getGroupNames().removeAll(group.getName()) > 0)
            {
                emit this->userChanged(i.value());
            }
        }
    }
}
