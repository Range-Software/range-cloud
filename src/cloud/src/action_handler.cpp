#include <QJsonDocument>
#include <QJsonArray>

#include <rbl_logger.h>
#include <rbl_utils.h>
#include <rcl_cloud_process_response.h>

#include "action_handler.h"

ActionHandler::ActionHandler(UserManager *userManager,
                             ActionManager *actionManager,
                             ProcessManager *processManager,
                             FileManager *fileManager,
                             ReportManager *reportManager,
                             Mailer *mailer,
                             QObject *parent)
    : QObject{parent}
    , userManager{userManager}
    , actionManager{actionManager}
    , processManager{processManager}
    , fileManager{fileManager}
    , reportManager{reportManager}
    , mailer{mailer}
{
    R_LOG_TRACE_IN;
    QObject::connect(this->fileManager,&FileManager::requestCompleted,this,&ActionHandler::onFileRequestCompleted);
    QObject::connect(this->processManager,&ProcessManager::processCompleted,this,&ActionHandler::onProcessRequestCompleted);
    R_LOG_TRACE_OUT;
}

void ActionHandler::resolveAction(const RCloudAction &action, const QString &from)
{
    R_LOG_TRACE_IN;

    RLogger::debug("[ActionHandler] Resolving action \"%s\".\n",action.getAction().toUtf8().constData());

    static QDateTime startTime = QDateTime::currentDateTimeUtc();

    QString executorUser = action.getExecutor();
    if (executorUser.isEmpty())
    {
        executorUser = RUserInfo::guestUser;
    }

    RUserInfo executorInfo(this->userManager->findUser(executorUser));

    if (executorInfo.isNull())
    {
        RCloudAction resolvedAction(action);
        QString responseMessage = QString("Invalid user. User \"%1\" is not valid.").arg(executorUser);
        RLogger::warning("[ActionHandler] %s\n",responseMessage.toUtf8().constData());
        resolvedAction.setData(responseMessage.toUtf8());
        resolvedAction.setErrorType(RError::InvalidInput);
        emit this->resolved(resolvedAction);
        R_LOG_TRACE_OUT;
        return;
    }

    if (!this->actionManager->authorizeUser(executorInfo,action.getAction()))
    {
        RCloudAction resolvedAction(action);
        QString responseMessage = QString("Unauthorized access. User \"%1\" is not alowed to execute action \"%2\".").arg(executorInfo.getName(),action.getAction());
        RLogger::warning("[ActionHandler] %s\n",responseMessage.toUtf8().constData());
        resolvedAction.setData(responseMessage.toUtf8());
        resolvedAction.setErrorType(RError::Unauthorized);
        emit this->resolved(resolvedAction);
        R_LOG_TRACE_OUT;
        return;
    }

    if (action.getAction() == RCloudAction::Action::Test::key)
    {
        RCloudAction resolvedAction(action);
        resolvedAction.setData(action.getData());
        resolvedAction.setErrorType(RError::None);
        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ListFiles::key)
    {
        FileObject *fileObject = new FileObject;

        QUuid requestId = this->fileManager->requestListFiles(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::FileInfo::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setId(action.getResourceId());

        QUuid requestId = this->fileManager->requestFileInfo(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::FileUpload::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setPath(action.getResourceName());
        fileObject->getInfo().setId(QUuid::createUuid());

        RAccessOwner accessOwner;
        accessOwner.setUser(executorInfo.getName());
        accessOwner.setGroup(RUserInfo::userGroup);

        RAccessMode accessMode;
        accessMode.setUserModeMask(RAccessMode::Mode::Read | RAccessMode::Mode::Write);
        accessMode.setGroupModeMask(RAccessMode::Mode::Read);
        accessMode.setOtherModeMask(RAccessMode::Mode::None);

        RAccessRights accessRights;
        accessRights.setOwner(accessOwner);
        accessRights.setMode(accessMode);

        fileObject->getInfo().setAccessRights(accessRights);

        fileObject->setContent(action.getData());

        QUuid requestId = this->fileManager->requestStoreFile(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::FileUpdate::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setPath(action.getResourceName());
        fileObject->getInfo().setId(action.getResourceId());

        fileObject->setContent(action.getData());

        QUuid requestId = this->fileManager->requestUpdateFile(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::FileUpdateAccessOwner::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setId(action.getResourceId());

        RAccessRights accessRights;
        accessRights.setOwner(RAccessOwner::fromJson(QJsonDocument::fromJson(action.getData()).object()));
        fileObject->getInfo().setAccessRights(accessRights);

        QUuid requestId = this->fileManager->requestUpdateFileAccessOwner(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::FileUpdateAccessMode::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setId(action.getResourceId());

        RAccessRights accessRights;
        accessRights.setMode(RAccessMode::fromJson(QJsonDocument::fromJson(action.getData()).object()));
        fileObject->getInfo().setAccessRights(accessRights);

        QUuid requestId = this->fileManager->requestUpdateFileAccessMode(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::FileUpdateVersion::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setId(action.getResourceId());

        fileObject->getInfo().setVersion(RVersion(QString(action.getData())));

        QUuid requestId = this->fileManager->requestUpdateFileVersion(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::FileUpdateTags::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setId(action.getResourceId());

        fileObject->getInfo().setTags(QString(action.getData()).split(','));

        QUuid requestId = this->fileManager->requestUpdateFileTags(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());

    }
    else if (action.getAction() == RCloudAction::Action::FileDownload::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setId(action.getResourceId());

        QUuid requestId = this->fileManager->requestRetrieveFile(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::FileRemove::key)
    {
        FileObject *fileObject = new FileObject;
        fileObject->getInfo().setId(action.getResourceId());

        QUuid requestId = this->fileManager->requestRemoveFile(executorInfo,fileObject);
        this->fileRequests.insert(requestId,action.getId());
    }
    else if (action.getAction() == RCloudAction::Action::Stop::key)
    {
        RCloudAction resolvedAction(action);
        resolvedAction.setData("Stop server triggered");
        resolvedAction.setErrorType(RError::None);
        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::Statistics::key)
    {
        RLogger::debug("[ActionHandler] Producting statistics\n");

        QJsonObject jGeneralObject;
        jGeneralObject["version"] = RVendor::version().toString();

        QDateTime currentTime = QDateTime::currentDateTimeUtc();
        qint64 elapsedDays = startTime.daysTo(currentTime);
        QTime elapsedTime = QTime(0,0).addSecs(startTime.secsTo(currentTime) - (elapsedDays * 24 * 60 * 60));

        QJsonObject jTimeObject;
        jTimeObject["start"] = startTime.toString(Qt::ISODate);
        jTimeObject["current"] = currentTime.toString(Qt::ISODate);
        jTimeObject["upTime"] = QString("%1 days, %2").arg(QString::number(elapsedDays),elapsedTime.toString(Qt::ISODate));

        RLogger::indent();
        QJsonArray jServicesArray;
        jServicesArray.append(this->fileManager->getStatisticsJson());
        jServicesArray.append(this->actionManager->getStatisticsJson());
        jServicesArray.append(this->processManager->getStatisticsJson());
        jServicesArray.append(this->reportManager->getStatisticsJson());
        jServicesArray.append(this->userManager->getStatisticsJson());
        jServicesArray.append(this->mailer->getStatisticsJson());
        RLogger::unindent();

        QJsonObject jObject;
        jObject["general"] = jGeneralObject;
        jObject["dateTime"] = jTimeObject;
        jObject["services"] = jServicesArray;

        RCloudAction resolvedAction(action);
        resolvedAction.setData(QJsonDocument(jObject).toJson());
        resolvedAction.setErrorType(RError::None);
        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::Process::key)
    {
        RCloudProcessRequest request(RCloudProcessRequest::fromJson(QJsonDocument::fromJson(action.getData()).object()));
        request.setExecutor(executorInfo);

        try
        {
            if (!this->processManager->containsProcess(request.getName()))
            {
                throw RError(RError::InvalidInput,R_ERROR_REF,QString("Invalid process. Process \"%1\" is not valid.").arg(request.getName()));
            }

            if (!this->processManager->authorizeUser(request.getExecutor(),request.getName()))
            {
                throw RError(RError::Unauthorized,R_ERROR_REF,QString("Unauthorized access. User \"%1\" is not alowed to execute process \"%2\".").arg(request.getExecutor().getName(),request.getName()));
            }

            QUuid requestId = this->processManager->submitProcess(request);
            this->processRequests.insert(requestId,action.getId());
        }
        catch (const RError &error)
        {
            RCloudProcessResponse response;
            response.setProcessRequest(request);
            response.setResponseMessage(error.getMessage());

            RCloudAction resolvedAction(action);
            resolvedAction.setData(QJsonDocument(response.toJson()).toJson());
            resolvedAction.setErrorType(error.getType());
            emit this->resolved(resolvedAction);
            R_LOG_TRACE_OUT;
            return;
        }
        catch (...)
        {
            RCloudAction resolvedAction(action);
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
            emit this->resolved(resolvedAction);
            R_LOG_TRACE_OUT;
            return;
        }
    }
    else if (action.getAction() == RCloudAction::Action::ListUsers::key)
    {
        QJsonObject jsonObject;
        QJsonArray jsonArray;
        for (const RUserInfo &userInfo : this->userManager->getUsers())
        {
            jsonArray.append(userInfo.toJson());
        }
        jsonObject["users"] = jsonArray;

        RCloudAction resolvedAction(action);
        resolvedAction.setData(QJsonDocument(jsonObject).toJson());
        resolvedAction.setErrorType(RError::None);
        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::UserInfo::key)
    {
        RCloudAction resolvedAction(action);

        if (this->userManager->containsUser(action.getResourceName()))
        {
            RUserInfo userInfo = this->userManager->findUser(action.getResourceName());
            resolvedAction.setData(QJsonDocument(userInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        else
        {
            resolvedAction.setData(RError::getTypeMessage(RError::NotFound).toUtf8());
            resolvedAction.setErrorType(RError::NotFound);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::UserAdd::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            RUserInfo userInfo(UserManager::createUser(action.getResourceName()));
            this->userManager->addUser(userInfo);
            resolvedAction.setData(QJsonDocument(userInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::UserUpdate::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            RUserInfo userInfo(RUserInfo::fromJson(QJsonDocument::fromJson(action.getData()).object()));
            this->userManager->setUser(action.getResourceName(),userInfo);
            resolvedAction.setData(QJsonDocument(userInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::UserRemove::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            this->userManager->removeUser(action.getResourceName());
            resolvedAction.setData(action.getResourceName().toUtf8());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ListUserTokens::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            if (!executorInfo.isUser(action.getResourceName()) &&
                !executorInfo.isUser(RUserInfo::rootUser) &&
                !executorInfo.hasGroup(RUserInfo::rootGroup))
            {
                QString message = QString("%1. User \"%2\" is not alowed to list authentication tokens with resource name \"%2\".").arg(
                    RError::getTypeMessage(RError::Unauthorized),
                    executorInfo.getName(),
                    action.getResourceName());
                throw RError(RError::Unauthorized,R_ERROR_REF,message);
            }

            QJsonObject jsonObject;
            QJsonArray jsonArray;
            for (const RAuthToken &authToken : this->userManager->getTokens())
            {
                if (authToken.getResourceName() == action.getResourceName())
                {
                    jsonArray.append(authToken.toJson());
                }
            }
            jsonObject["tokens"] = jsonArray;

            resolvedAction.setData(QJsonDocument(jsonObject).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::UserTokenGenerate::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            if (!executorInfo.isUser(action.getResourceName()) &&
                !executorInfo.isUser(RUserInfo::rootUser) &&
                !executorInfo.hasGroup(RUserInfo::rootGroup))
            {
                QString message = QString("%1. User \"%2\" is not alowed to generate authentication token with resource name \"%2\".").arg(
                    RError::getTypeMessage(RError::Unauthorized),
                    executorInfo.getName(),
                    action.getResourceName());
                throw RError(RError::Unauthorized,R_ERROR_REF,message);
            }

            RAuthToken authToken;
            authToken.setResourceName(action.getResourceName());
            authToken.setContent(RAuthToken::generateTokenContent());
            authToken.setValidityDate(RAuthToken::validityMonthsFromNow(1));

            this->userManager->addToken(authToken);
            resolvedAction.setData(QJsonDocument(authToken.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);

            QString message = QString("New authentication token has been crated.\n\nResource: %1\nToken: %2\nValidity: %3").arg(
                authToken.getResourceName(),
                authToken.getContent(),
                QDateTime::fromSecsSinceEpoch(authToken.getValidityDate()).toString());
            this->mailer->submitMail(authToken.getResourceName(),"Authentication token created", message);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::UserTokenRemove::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            if (!executorInfo.isUser(action.getResourceName()) &&
                !executorInfo.isUser(RUserInfo::rootUser) &&
                !executorInfo.hasGroup(RUserInfo::rootGroup))
            {
                QString message = QString("%1. User \"%2\" is not alowed to remove authentication token with resource name \"%2\".").arg(
                    RError::getTypeMessage(RError::Unauthorized),
                    executorInfo.getName(),
                    action.getResourceName());
                throw RError(RError::Unauthorized,R_ERROR_REF,message);
            }

            this->userManager->removeToken(action.getResourceId());
            resolvedAction.setData(action.getResourceId().toString(QUuid::WithoutBraces).toUtf8());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ListGroups::key)
    {
        QJsonObject jsonObject;
        QJsonArray jsonArray;
        for (const RGroupInfo &groupInfo : this->userManager->getGroups())
        {
            jsonArray.append(groupInfo.toJson());
        }
        jsonObject["groups"] = jsonArray;

        RCloudAction resolvedAction(action);
        resolvedAction.setData(QJsonDocument(jsonObject).toJson());
        resolvedAction.setErrorType(RError::None);
        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::GroupInfo::key)
    {
        RCloudAction resolvedAction(action);

        if (this->userManager->containsGroup(action.getResourceName()))
        {
            RGroupInfo groupInfo = this->userManager->findGroup(action.getResourceName());
            resolvedAction.setData(QJsonDocument(groupInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        else
        {
            resolvedAction.setData(RError::getTypeMessage(RError::NotFound).toUtf8());
            resolvedAction.setErrorType(RError::NotFound);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::GroupAdd::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            RGroupInfo groupInfo(UserManager::createGroup(action.getResourceName()));
            this->userManager->addGroup(groupInfo);
            resolvedAction.setData(QJsonDocument(groupInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::GroupRemove::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            this->userManager->removeGroup(action.getResourceName());
            resolvedAction.setData(action.getResourceName().toUtf8());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ListActions::key)
    {
        QJsonObject jsonObject;
        QJsonArray jsonArray;
        for (const RCloudActionInfo &actionInfo : this->actionManager->getActions())
        {
            jsonArray.append(actionInfo.toJson());
        }
        jsonObject["actions"] = jsonArray;

        RCloudAction resolvedAction(action);
        resolvedAction.setData(QJsonDocument(jsonObject).toJson());
        resolvedAction.setErrorType(RError::None);
        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ActionUpdateAccessOwner::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            RCloudActionInfo actionInfo = this->actionManager->findAction(action.getResourceName());

            RAccessRights accessRights = actionInfo.getAccessRights();
            accessRights.setOwner(RAccessOwner::fromJson(QJsonDocument::fromJson(action.getData()).object()));
            actionInfo.setAccessRights(accessRights);

            actionInfo = this->actionManager->updateActionAccessRights(action.getResourceName(),accessRights);
            resolvedAction.setData(QJsonDocument(actionInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ActionUpdateAccessMode::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            RCloudActionInfo actionInfo = this->actionManager->findAction(action.getResourceName());

            RAccessRights accessRights = actionInfo.getAccessRights();
            accessRights.setMode(RAccessMode::fromJson(QJsonDocument::fromJson(action.getData()).object()));
            actionInfo.setAccessRights(accessRights);

            actionInfo = this->actionManager->updateActionAccessRights(action.getResourceName(),accessRights);
            resolvedAction.setData(QJsonDocument(actionInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ListProcesses::key)
    {
        QJsonObject jsonObject;
        QJsonArray jsonArray;
        for (const RCloudProcessInfo &processInfo : this->processManager->getProcesses())
        {
            jsonArray.append(processInfo.toJson());
        }
        jsonObject["processes"] = jsonArray;

        RCloudAction resolvedAction(action);
        resolvedAction.setData(QJsonDocument(jsonObject).toJson());
        resolvedAction.setErrorType(RError::None);
        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ProcessUpdateAccessOwner::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            RCloudProcessInfo processInfo = this->processManager->findProcess(action.getResourceName());

            RAccessRights accessRights = processInfo.getAccessRights();
            accessRights.setOwner(RAccessOwner::fromJson(QJsonDocument::fromJson(action.getData()).object()));
            processInfo.setAccessRights(accessRights);

            processInfo = this->processManager->updateProcessAccessRights(action.getResourceName(),accessRights);
            resolvedAction.setData(QJsonDocument(processInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::ProcessUpdateAccessMode::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            RCloudProcessInfo processInfo = this->processManager->findProcess(action.getResourceName());

            RAccessRights accessRights = processInfo.getAccessRights();
            accessRights.setMode(RAccessMode::fromJson(QJsonDocument::fromJson(action.getData()).object()));
            processInfo.setAccessRights(accessRights);

            processInfo = this->processManager->updateProcessAccessRights(action.getResourceName(),accessRights);
            resolvedAction.setData(QJsonDocument(processInfo.toJson()).toJson());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else if (action.getAction() == RCloudAction::Action::SubmitReport::key)
    {
        RCloudAction resolvedAction(action);

        try
        {
            QUuid id = this->reportManager->submitReport(from,RReportRecord::fromJson(QJsonDocument::fromJson(action.getData()).object()));
            resolvedAction.setData(QString("Report (id=%1) has been stored.").arg(id.toString(QUuid::WithoutBraces)).toUtf8());
            resolvedAction.setErrorType(RError::None);
        }
        catch (const RError &error)
        {
            resolvedAction.setData(error.getMessage().toUtf8());
            resolvedAction.setErrorType(error.getType());
        }
        catch (...)
        {
            resolvedAction.setData(RError::getTypeMessage(RError::Unknown).toUtf8());
            resolvedAction.setErrorType(RError::Unknown);
        }

        emit this->resolved(resolvedAction);
    }
    else
    {
        RLogger::error("[ActionHandler] Unknown private action: \"%s\"\n",action.getAction().toUtf8().constData());
    }
    R_LOG_TRACE_OUT;
}

void ActionHandler::onFileRequestCompleted(const QUuid &requestId, const FileObject *object)
{
    R_LOG_TRACE_IN;
    RLogger::info("[ActionHandler] File request ID: \"%s\" completed with error type: \"%d - %s\".\n",
                  requestId.toString(QUuid::WithoutBraces).toUtf8().constData(),
                  object->getErrorType(),
                  RError::getTypeMessage(object->getErrorType()).toUtf8().constData());
    if (!this->fileRequests.contains(requestId))
    {
        RLogger::info("[ActionHandler] File request \"%s\" not found among registered requests.\n",requestId.toString(QUuid::WithoutBraces).toUtf8().constData());
    }
    else
    {
        RCloudAction action(this->fileRequests.value(requestId),
                       object->getInfo().getAccessRights().getOwner().getUser(),
                       QString(),
                       QString(),
                       object->getInfo().getPath(),
                       object->getInfo().getId(),
                       object->getContent());
        action.setErrorType(object->getErrorType());
        this->fileRequests.remove(requestId);
        delete object;
        emit this->resolved(action);
    }
    R_LOG_TRACE_OUT;
}

void ActionHandler::onProcessRequestCompleted(const QUuid &requestId, const RCloudProcessResult &result)
{
    R_LOG_TRACE_IN;
    RLogger::info("[ActionHandler] Process request ID: \"%s\" completed with error type: \"%d - %s\".\n",
                  requestId.toString(QUuid::WithoutBraces).toUtf8().constData(),
                  result.getErrorType(),
                  RError::getTypeMessage(result.getErrorType()).toUtf8().constData());
    if (!this->processRequests.contains(requestId))
    {
        RLogger::info("[ActionHandler] Process request \"%s\" not found among registered requests.\n",requestId.toString(QUuid::WithoutBraces).toUtf8().constData());
    }
    else
    {
        RCloudProcessResponse response;
        response.setProcessRequest(result.getProcessRequest());
        response.setResponseMessage(result.getErrorType() == RError::None ? result.getOutputBuffer() : result.getErrorBuffer());
        RCloudAction action(this->processRequests.value(requestId),
                       QString(),
                       QString(),
                       QString(),
                       QString(),
                       QUuid(),
                       QJsonDocument(response.toJson()).toJson());
        action.setErrorType(result.getErrorType());
        this->processRequests.remove(requestId);
        this->processManager->finalizeProcess(requestId);
        emit this->resolved(action);
    }
    R_LOG_TRACE_OUT;
}
