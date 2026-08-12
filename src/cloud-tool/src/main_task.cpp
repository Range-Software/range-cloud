#include <QSettings>
#include <QDate>
#include <QFileInfo>
#include <QDir>
#include <QTimer>

#include <rbl_error.h>
#include <rbl_file_tools.h>
#include <rbl_job_manager.h>
#include <rbl_logger.h>
#include <rbl_tool_task.h>

#include <rcl_cloud_action.h>
#include <rcl_cloud_tool_action.h>

#include "main_task.h"

MainTask::MainTask(Application *application)
    : QObject(application)
    , application(application)
    , aiQueryInFlight(false)
    , aiQueryPollCount(0)
{
    R_LOG_TRACE_IN;
    R_LOG_TRACE_OUT;
}

void MainTask::run()
{
    R_LOG_TRACE_IN;
    try
    {
        // Start tool.
        RToolTask *toolTask = new RToolTask(this->application->getToolInput());
        toolTask->setBlocking(false);

        QObject::connect(toolTask, &RToolTask::actionFinished, this, &MainTask::actionFinished);
        QObject::connect(toolTask, &RToolTask::actionFailed, this, &MainTask::actionFailed);
        QObject::connect(toolTask, &RToolTask::finished, this, &MainTask::taskFinished);
        QObject::connect(toolTask, &RToolTask::failed, this, &MainTask::taskFailed);

        RJobManager::getInstance().submit(toolTask);
    }
    catch (const RError &error)
    {
        RLogger::error("Failed to start tool. %s\n",error.getMessage().toUtf8().constData());
        this->application->exit(1);
    }
    R_LOG_TRACE_OUT;
}

void MainTask::actionFinished(const QSharedPointer<RToolAction> &action)
{
    R_LOG_TRACE_IN;
    RHttpMessage responseMessage = action.staticCast<RCloudToolAction>().data()->getResponseMessage();
    if (action.staticCast<RCloudToolAction>().data()->getType() == RCloudToolAction::AIQueryResult)
    {
        // Result polls repeat until the query settles; keep them quiet.
        RLogger::debug("Action has finished.\n");
    }
    else
    {
        RLogger::info("Action has finished.\n");
    }

    const QString &outputFileName = Application::instance()->getOutputFileName();

    switch (action.staticCast<RCloudToolAction>().data()->getType())
    {
        case RCloudToolAction::Test:
        {
            if (!outputFileName.isEmpty())
            {
                if (!RFileTools::writeAsciiFile(outputFileName,responseMessage.getBody().constData()))
                {
                    RLogger::error("Failed to write action output to file \"%s\".\n", outputFileName.toUtf8().constData());
                }
            }
            else
            {
                RLogger::info("Connection test response:\n");
                RLogger::info("%s\n",RCloudToolAction::processTestResponse(responseMessage.getBody()).toUtf8().constData());
            }

            break;
        }
        case RCloudToolAction::FileDownload:
        {
            QString path = responseMessage.getProperties()[RCloudAction::Resource::Name::key];
            if (!RFileTools::writeBinaryFile(path,responseMessage.getBody()))
            {
                RLogger::error("Failed to write downloaded file \"%s\".\n", path.toUtf8().constData());
            }
            break;
        }
        case RCloudToolAction::AIQuery:
        case RCloudToolAction::AIQueryResult:
        {
            RCloudAIQueryResponse aiQueryResponse = RCloudToolAction::processAIQueryResult(responseMessage.getBody());
            if (aiQueryResponse.getStatus() == RCloudAIQueryResponse::Pending)
            {
                // The query is answered asynchronously: keep polling the
                // server for the result until it settles.
                if (action.staticCast<RCloudToolAction>().data()->getType() == RCloudToolAction::AIQuery)
                {
                    RLogger::info("AI query submitted, id = \"%s\". Waiting for result.\n",
                                  aiQueryResponse.getId().toString(QUuid::WithoutBraces).toUtf8().constData());
                }
                if (this->aiQueryPollCount >= MainTask::aiQueryMaxPolls)
                {
                    RLogger::error("AI query id = \"%s\" did not complete in time. Giving up.\n",
                                   aiQueryResponse.getId().toString(QUuid::WithoutBraces).toUtf8().constData());
                    this->aiQueryInFlight = false;
                    break;
                }
                this->aiQueryInFlight = true;
                this->aiQueryPollCount++;
                this->scheduleAIQueryResultPoll(aiQueryResponse.getId());
            }
            else
            {
                this->aiQueryInFlight = false;
                const QString &aiResponse = aiQueryResponse.getResponseMessage();
                if (!outputFileName.isEmpty())
                {
                    if (!RFileTools::writeAsciiFile(outputFileName,aiResponse.toUtf8().constData()))
                    {
                        RLogger::error("Failed to write action output to file \"%s\".\n", outputFileName.toUtf8().constData());
                    }
                }
                else
                {
                    RLogger::info("AI query response:\n");
                    RLogger::info("%s\n",aiResponse.toUtf8().constData());
                }
            }
            break;
        }
        default:
        {
            if (!outputFileName.isEmpty())
            {
                if (!RFileTools::writeAsciiFile(outputFileName,responseMessage.getBody().constData()))
                {
                    RLogger::error("Failed to write action output to file \"%s\".\n", outputFileName.toUtf8().constData());
                }
            }
            else
            {
                RLogger::info("Unhandled action response:\n");
                RLogger::info("%s\n",responseMessage.getBody().constData());
            }
        }
    }

    R_LOG_TRACE_OUT;
}

void MainTask::actionFailed(const QSharedPointer<RToolAction> &action)
{
    R_LOG_TRACE_IN;
    RHttpMessage responseMessage = action.staticCast<RCloudToolAction>().data()->getResponseMessage();
    RLogger::info("Action has failed.\n");
    this->aiQueryInFlight = false;
    if (action.staticCast<RCloudToolAction>().data()->getType() != RCloudToolAction::FileDownload)
    {
        RLogger::info("Unhandled action response:\n");
        RLogger::info("%s\n",responseMessage.getBody().constData());
    }
    R_LOG_TRACE_OUT;
}

void MainTask::scheduleAIQueryResultPoll(const QUuid &requestId)
{
    QTimer::singleShot(MainTask::aiQueryPollIntervalMs, this, [this,requestId]() {
        try
        {
            RToolInput toolInput;
            toolInput.addAction(RCloudToolAction::requestAIQueryResult(this->application->getHttpClient(),
                                                                       requestId,
                                                                       this->application->getAuthUser(),
                                                                       this->application->getAuthToken()));

            RToolTask *toolTask = new RToolTask(toolInput);
            toolTask->setBlocking(false);

            QObject::connect(toolTask, &RToolTask::actionFinished, this, &MainTask::actionFinished);
            QObject::connect(toolTask, &RToolTask::actionFailed, this, &MainTask::actionFailed);
            QObject::connect(toolTask, &RToolTask::finished, this, &MainTask::taskFinished);
            QObject::connect(toolTask, &RToolTask::failed, this, &MainTask::taskFailed);

            RJobManager::getInstance().submit(toolTask);
        }
        catch (const RError &error)
        {
            RLogger::error("Failed to poll AI query result. %s\n",error.getMessage().toUtf8().constData());
            this->aiQueryInFlight = false;
            this->application->disconnect();
        }
    });
}

void MainTask::taskFinished()
{
    R_LOG_TRACE_IN;
    if (this->aiQueryInFlight)
    {
        // An AI query result poll is scheduled; keep the application running.
        RLogger::debug("Task has finished.\n");
        R_LOG_TRACE_OUT;
        return;
    }
    RLogger::info("Task has finished.\n");
    this->application->disconnect();
    R_LOG_TRACE_OUT;
}

void MainTask::taskFailed()
{
    R_LOG_TRACE_IN;
    RLogger::info("Task has failed.\n");
    this->application->disconnect();
    R_LOG_TRACE_OUT;
}
