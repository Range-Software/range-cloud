#include <rcl_report_record.h>

#include "report_manager_settings.h"

void ReportManagerSettings::_init(const ReportManagerSettings *pReportManagerSettings)
{
    this->ServiceSettings::_init(pReportManagerSettings);
    if (pReportManagerSettings)
    {
        this->reportDirectory = pReportManagerSettings->reportDirectory;
        this->maxReportLength = pReportManagerSettings->maxReportLength;
        this->maxCommentLength = pReportManagerSettings->maxCommentLength;
    }
}

ReportManagerSettings::ReportManagerSettings()
    : maxReportLength{RReportRecord::defaultMaxReportLength}
    , maxCommentLength{RReportRecord::defaultMaxCommentLength}
{
    this->_init();
    this->name = "ReportService";
}

ReportManagerSettings::ReportManagerSettings(const ReportManagerSettings &reportManagerSettings)
    : ServiceSettings(reportManagerSettings)
{
    this->_init(&reportManagerSettings);
}

ReportManagerSettings &ReportManagerSettings::operator =(const ReportManagerSettings &reportManagerSettings)
{
    this->_init(&reportManagerSettings);
    return (*this);
}

const QString &ReportManagerSettings::getReportDirectory() const
{
    return this->reportDirectory;
}

void ReportManagerSettings::setReportDirectory(const QString &reportDirectory)
{
    this->reportDirectory = reportDirectory;
}

qint64 ReportManagerSettings::getMaxReportLength() const
{
    return this->maxReportLength;
}

void ReportManagerSettings::setMaxReportLength(qint64 maxReportLength)
{
    this->maxReportLength = maxReportLength;
}

qint64 ReportManagerSettings::getMaxCommentLength() const
{
    return this->maxCommentLength;
}

void ReportManagerSettings::setMaxCommentLength(qint64 maxCommentLength)
{
    this->maxCommentLength = maxCommentLength;
}
