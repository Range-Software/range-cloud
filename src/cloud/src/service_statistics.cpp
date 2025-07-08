#include <rbl_statistics.h>

#include "service_statistics.h"
#include <QJsonArray>

void ServiceStatistics::_init(const ServiceStatistics *pServiceStatistics)
{
    if (pServiceStatistics)
    {
        this->name = pServiceStatistics->name;
        this->dataCounter = pServiceStatistics->dataCounter;
        this->dataSetMap = pServiceStatistics->dataSetMap;
    }
}

ServiceStatistics::ServiceStatistics()
{
    this->_init();
}

ServiceStatistics::ServiceStatistics(const ServiceStatistics &serviceStatistics)
{
    this->_init(&serviceStatistics);
}

ServiceStatistics &ServiceStatistics::operator =(const ServiceStatistics &serviceStatistics)
{
    this->_init(&serviceStatistics);
    return (*this);
}

const QString &ServiceStatistics::getName() const
{
    return this->name;
}

void ServiceStatistics::setName(const QString &name)
{
    this->name = name;
}

void ServiceStatistics::clear()
{
    this->dataCounter.clear();
    this->dataSetMap.clear();
}

void ServiceStatistics::recordCounter(const QString &key, qsizetype counter)
{
    this->dataCounter[key] += counter;
}

void ServiceStatistics::recordValue(const QString &key, double value)
{
    this->dataSetMap[key].push_back(value);
}

void ServiceStatistics::setValues(const QString &key, const RRVector &values)
{
    this->dataSetMap[key] = values;
}

QJsonObject ServiceStatistics::toJson() const
{
    QJsonObject jObject;

    jObject["name"] = this->name;
    for (auto iter = this->dataCounter.cbegin(); iter != this->dataCounter.cend(); ++iter)
    {
        jObject[iter.key()] = iter.value();
    }
    for (auto iter = this->dataSetMap.cbegin(); iter != this->dataSetMap.cend(); ++iter)
    {
        jObject[iter.key()] = RStatistics(iter.value()).toJson();
    }

    return jObject;
}
