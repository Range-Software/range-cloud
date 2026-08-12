#include "ai_query_manager_settings.h"

void AIQueryManagerSettings::_init(const AIQueryManagerSettings *pAIQueryManagerSettings)
{
    this->ServiceSettings::_init(pAIQueryManagerSettings);
    if (pAIQueryManagerSettings)
    {
        this->type = pAIQueryManagerSettings->type;
        this->apiUrl = pAIQueryManagerSettings->apiUrl;
        this->apiKey = pAIQueryManagerSettings->apiKey;
        this->model = pAIQueryManagerSettings->model;
        this->maxTokens = pAIQueryManagerSettings->maxTokens;
        this->timeout = pAIQueryManagerSettings->timeout;
        this->maxFileContextSize = pAIQueryManagerSettings->maxFileContextSize;
        this->remoteFileStoreSize = pAIQueryManagerSettings->remoteFileStoreSize;
        this->aiQueriesFileName = pAIQueryManagerSettings->aiQueriesFileName;
        this->fileStoreIndexFileName = pAIQueryManagerSettings->fileStoreIndexFileName;
    }
}

AIQueryManagerSettings::AIQueryManagerSettings()
    : maxTokens{1024}
    , timeout{60000}
    , maxFileContextSize{256 * 1024}
    , remoteFileStoreSize{100LL * 1024 * 1024 * 1024}
{
    this->_init();
    this->name = "AIQueryService";
}

AIQueryManagerSettings::AIQueryManagerSettings(const AIQueryManagerSettings &aiQueryManagerSettings)
    : ServiceSettings(aiQueryManagerSettings)
{
    this->_init(&aiQueryManagerSettings);
}

AIQueryManagerSettings &AIQueryManagerSettings::operator =(const AIQueryManagerSettings &aiQueryManagerSettings)
{
    this->_init(&aiQueryManagerSettings);
    return (*this);
}

const QString &AIQueryManagerSettings::getType() const
{
    return this->type;
}

void AIQueryManagerSettings::setType(const QString &type)
{
    this->type = type;
}

const QString &AIQueryManagerSettings::getApiUrl() const
{
    return this->apiUrl;
}

void AIQueryManagerSettings::setApiUrl(const QString &apiUrl)
{
    this->apiUrl = apiUrl;
}

const QString &AIQueryManagerSettings::getApiKey() const
{
    return this->apiKey;
}

void AIQueryManagerSettings::setApiKey(const QString &apiKey)
{
    this->apiKey = apiKey;
}

const QString &AIQueryManagerSettings::getModel() const
{
    return this->model;
}

void AIQueryManagerSettings::setModel(const QString &model)
{
    this->model = model;
}

qint64 AIQueryManagerSettings::getMaxTokens() const
{
    return this->maxTokens;
}

void AIQueryManagerSettings::setMaxTokens(qint64 maxTokens)
{
    this->maxTokens = maxTokens;
}

qint64 AIQueryManagerSettings::getTimeout() const
{
    return this->timeout;
}

void AIQueryManagerSettings::setTimeout(qint64 timeout)
{
    this->timeout = timeout;
}

qint64 AIQueryManagerSettings::getMaxFileContextSize() const
{
    return this->maxFileContextSize;
}

void AIQueryManagerSettings::setMaxFileContextSize(qint64 maxFileContextSize)
{
    this->maxFileContextSize = maxFileContextSize;
}

qint64 AIQueryManagerSettings::getRemoteFileStoreSize() const
{
    return this->remoteFileStoreSize;
}

void AIQueryManagerSettings::setRemoteFileStoreSize(qint64 remoteFileStoreSize)
{
    this->remoteFileStoreSize = remoteFileStoreSize;
}

const QString &AIQueryManagerSettings::getAiQueriesFileName() const
{
    return this->aiQueriesFileName;
}

void AIQueryManagerSettings::setAiQueriesFileName(const QString &aiQueriesFileName)
{
    this->aiQueriesFileName = aiQueriesFileName;
}

const QString &AIQueryManagerSettings::getFileStoreIndexFileName() const
{
    return this->fileStoreIndexFileName;
}

void AIQueryManagerSettings::setFileStoreIndexFileName(const QString &fileStoreIndexFileName)
{
    this->fileStoreIndexFileName = fileStoreIndexFileName;
}
