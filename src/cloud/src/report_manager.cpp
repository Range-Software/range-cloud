#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>

#include <rbl_error.h>
#include <rbl_logger.h>

#include "report_manager.h"

ReportManager::ReportManager(const ReportManagerSettings &settings, QObject *parent)
    : QObject{parent}
    , settings{settings}
    , nReportsSubmitted{0}
{
    R_LOG_TRACE_IN;
    this->statistics.setName(this->settings.getName());
    R_LOG_TRACE_OUT;
}

QUuid ReportManager::submitReport(const QString &from, const RReportRecord &reportRecord)
{
    R_LOG_TRACE_IN;
    if (this->settings.getMaxReportLength() >= 0 && reportRecord.getReport().size() > this->settings.getMaxReportLength())
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::InvalidInput, R_ERROR_REF,
                     "Report length '%u' is bigger then maximum allowed '%u'.",
                     reportRecord.getReport().size(),
                     this->settings.getMaxReportLength());
    }
    if (this->settings.getMaxCommentLength() >= 0 && reportRecord.getComment().size() > this->settings.getMaxCommentLength())
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::InvalidInput, R_ERROR_REF,
                     "Comment length '%u' is bigger then maximum allowed '%u'.",
                     reportRecord.getComment().size(),
                     this->settings.getMaxCommentLength());
    }

    QUuid id = QUuid::createUuid();

    QDateTime timeStamp = QDateTime::currentDateTime();

    QDir reportDir(this->settings.getReportDirectory());
    QString reportFile = reportDir.absoluteFilePath(QString("%1-%2.rpt").arg(timeStamp.toString("yyyyMMdd-hhmmss"),id.toString(QUuid::WithoutBraces)));

    RLogger::info("[%s] Writing report file \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),reportFile.toUtf8().constData());

    QFile outFile(reportFile);

    if(!outFile.open(QIODevice::WriteOnly))
    {
        R_LOG_TRACE_OUT;
        throw RError(RError::Type::OpenFile,R_ERROR_REF,
                     "Failed to open users file \"%s\" for writing. %s.",
                     outFile.fileName().toUtf8().constData(),
                     outFile.errorString().toUtf8().constData());
    }

    qint64 bytesOut = 0;

    bytesOut += outFile.write("ID: ");
    bytesOut += outFile.write(id.toString(QUuid::WithoutBraces).toUtf8());
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("FROM: ");
    bytesOut += outFile.write(from.toUtf8());
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("CREATED: ");
    bytesOut += outFile.write(QDateTime::fromSecsSinceEpoch(reportRecord.getCreationDateTime()).toString().toUtf8());
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("RECORDED: ");
    bytesOut += outFile.write(QDateTime::currentDateTime().toString().toUtf8());
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("================================================================================\n");
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("REPORT BEGIN\n");
    bytesOut += outFile.write("--------------------------------------------------------------------------------\n");
    bytesOut += outFile.write(reportRecord.getReport().toUtf8());
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("--------------------------------------------------------------------------------\n");
    bytesOut += outFile.write("REPORT END\n");
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("================================================================================\n");
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("COMMENT BEGIN\n");
    bytesOut += outFile.write("--------------------------------------------------------------------------------\n");
    bytesOut += outFile.write(reportRecord.getComment().toUtf8());
    bytesOut += outFile.write("\n");
    bytesOut += outFile.write("--------------------------------------------------------------------------------\n");
    bytesOut += outFile.write("COMMENT END\n");

    RLogger::info("[%s] Successfuly wrote \"%ld\" bytes to \"%s\".\n",
                  this->settings.getName().toUtf8().constData(),bytesOut,outFile.fileName().toUtf8().constData());

    outFile.close();

    this->nReportsSubmitted++;

    R_LOG_TRACE_RETURN(id);
}

QJsonObject ReportManager::getStatisticsJson() const
{
    R_LOG_TRACE_IN;
    RLogger::debug("[%s] Producting statistics\n",this->settings.getName().toUtf8().constData());
    ServiceStatistics snapshotStatistics(this->statistics);
    snapshotStatistics.recordCounter("reports",this->nReportsSubmitted);
    R_LOG_TRACE_RETURN(snapshotStatistics.toJson());
}

