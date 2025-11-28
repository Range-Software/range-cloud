#include <QSettings>
#include <QDate>
#include <QFileInfo>
#include <QDir>

#include <rbl_error.h>
#include <rbl_job_manager.h>
#include <rbl_logger.h>
#include <rbl_tool_task.h>

#include <rcl_cloud_action.h>
#include <rcl_cloud_tool_action.h>
#include <rcl_file_tools.h>

#include "main_task.h"

MainTask::MainTask(Application *application)
    : QObject(application)
    , application(application)
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
    RLogger::info("Action has finished.\n");

    switch (action.staticCast<RCloudToolAction>().data()->getType())
    {
        case RCloudToolAction::Test:
        {
            RLogger::info("Connection test response:\n");
            RLogger::info("%s\n",RCloudToolAction::processTestResponse(responseMessage.getBody()).toUtf8().constData());
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
        default:
        {
            RLogger::info("Unhandled action response:\n");
            RLogger::info("%s\n",responseMessage.getBody().constData());
        }
    }

    R_LOG_TRACE_OUT;
}

void MainTask::actionFailed(const QSharedPointer<RToolAction> &action)
{
    R_LOG_TRACE_IN;
    RHttpMessage responseMessage = action.staticCast<RCloudToolAction>().data()->getResponseMessage();
    RLogger::info("Action has failed.\n");
    if (action.staticCast<RCloudToolAction>().data()->getType() != RCloudToolAction::FileDownload)
    {
        RLogger::info("Unhandled action response:\n");
        RLogger::info("%s\n",responseMessage.getBody().constData());
    }
    R_LOG_TRACE_OUT;
}

void MainTask::taskFinished()
{
    R_LOG_TRACE_IN;
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
