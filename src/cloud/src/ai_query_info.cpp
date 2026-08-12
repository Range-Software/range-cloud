#include <QJsonArray>

#include "ai_query_info.h"

void AIQueryInfo::_init(const AIQueryInfo *pAIQueryInfo)
{
    if (pAIQueryInfo)
    {
        this->name = pAIQueryInfo->name;
        this->aliases = pAIQueryInfo->aliases;
        this->systemPrompt = pAIQueryInfo->systemPrompt;
    }
}

AIQueryInfo::AIQueryInfo()
{
    this->_init();
}

AIQueryInfo::AIQueryInfo(const AIQueryInfo &aiQueryInfo)
{
    this->_init(&aiQueryInfo);
}

AIQueryInfo::~AIQueryInfo()
{

}

AIQueryInfo &AIQueryInfo::operator =(const AIQueryInfo &aiQueryInfo)
{
    this->_init(&aiQueryInfo);
    return (*this);
}

const QString &AIQueryInfo::getName() const
{
    return this->name;
}

void AIQueryInfo::setName(const QString &name)
{
    this->name = name;
}

const QStringList &AIQueryInfo::getAliases() const
{
    return this->aliases;
}

void AIQueryInfo::setAliases(const QStringList &aliases)
{
    this->aliases = aliases;
}

const QString &AIQueryInfo::getSystemPrompt() const
{
    return this->systemPrompt;
}

void AIQueryInfo::setSystemPrompt(const QString &systemPrompt)
{
    this->systemPrompt = systemPrompt;
}

bool AIQueryInfo::matches(const QString &application) const
{
    const QString key = application.trimmed().toLower();
    if (key == this->name.trimmed().toLower())
    {
        return true;
    }
    foreach (const QString &alias, this->aliases)
    {
        if (key == alias.trimmed().toLower())
        {
            return true;
        }
    }
    return false;
}

AIQueryInfo AIQueryInfo::fromJson(const QJsonObject &json)
{
    AIQueryInfo aiQueryInfo;

    if (const QJsonValue &v = json["name"]; v.isString())
    {
        aiQueryInfo.name = v.toString();
    }

    if (const QJsonValue &v = json["aliases"]; v.isArray())
    {
        const QJsonArray &aliases = v.toArray();

        aiQueryInfo.aliases.reserve(aliases.size());
        for (const QJsonValue &alias : aliases)
        {
            if (alias.isString())
            {
                aiQueryInfo.aliases.append(alias.toString());
            }
        }
    }

    if (const QJsonValue &v = json["systemPrompt"]; v.isString())
    {
        aiQueryInfo.systemPrompt = v.toString();
    }

    return aiQueryInfo;
}

QJsonObject AIQueryInfo::toJson() const
{
    QJsonObject json;

    json["name"] = this->name;

    QJsonArray aliasesArray;
    for (const QString &alias : this->aliases)
    {
        aliasesArray.append(alias);
    }
    json["aliases"] = aliasesArray;

    json["systemPrompt"] = this->systemPrompt;

    return json;
}
