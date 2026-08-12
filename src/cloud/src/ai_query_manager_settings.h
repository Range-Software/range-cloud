#ifndef AI_QUERY_MANAGER_SETTINGS_H
#define AI_QUERY_MANAGER_SETTINGS_H

#include <QString>

#include "service_settings.h"

class AIQueryManagerSettings : public ServiceSettings
{

    protected:

        //! Internal initialization function.
        void _init(const AIQueryManagerSettings *pAIQueryManagerSettings = nullptr);

    protected:

        //! AI agent type (e.g. "Anthropic", "OpenAI"); see RAgentSettings::typeToString.
        QString type;
        //! AI API endpoint URL.
        QString apiUrl;
        //! AI API authentication key.
        QString apiKey;
        //! Default model name.
        QString model;
        //! Maximum number of tokens to generate.
        qint64 maxTokens;
        //! Request timeout in milliseconds.
        qint64 timeout;
        //! Maximum size of a file embedded into query context in bytes.
        qint64 maxFileContextSize;
        //! Capacity of the remote (Anthropic) file store in bytes.
        qint64 remoteFileStoreSize;
        //! AI queries file name.
        QString aiQueriesFileName;
        //! Remote file store index file name.
        QString fileStoreIndexFileName;

    public:

        //! Constructor.
        AIQueryManagerSettings();

        //! Copy constructor.
        AIQueryManagerSettings(const AIQueryManagerSettings &aiQueryManagerSettings);

        //! Destructor.
        ~AIQueryManagerSettings() {}

        //! Assignment operator.
        AIQueryManagerSettings &operator =(const AIQueryManagerSettings &aiQueryManagerSettings);

        //! Get const reference to agent type.
        const QString &getType() const;

        //! Set new agent type.
        void setType(const QString &type);

        //! Get const reference to API URL.
        const QString &getApiUrl() const;

        //! Set new API URL.
        void setApiUrl(const QString &apiUrl);

        //! Get const reference to API key.
        const QString &getApiKey() const;

        //! Set new API key.
        void setApiKey(const QString &apiKey);

        //! Get const reference to default model name.
        const QString &getModel() const;

        //! Set new default model name.
        void setModel(const QString &model);

        //! Get maximum number of tokens to generate.
        qint64 getMaxTokens() const;

        //! Set maximum number of tokens to generate.
        void setMaxTokens(qint64 maxTokens);

        //! Get request timeout in milliseconds.
        qint64 getTimeout() const;

        //! Set request timeout in milliseconds.
        void setTimeout(qint64 timeout);

        //! Get maximum size of a file embedded into query context in bytes.
        qint64 getMaxFileContextSize() const;

        //! Set maximum size of a file embedded into query context in bytes.
        void setMaxFileContextSize(qint64 maxFileContextSize);

        //! Get capacity of the remote (Anthropic) file store in bytes.
        qint64 getRemoteFileStoreSize() const;

        //! Set capacity of the remote (Anthropic) file store in bytes.
        void setRemoteFileStoreSize(qint64 remoteFileStoreSize);

        //! Get const reference to AI queries file name.
        const QString &getAiQueriesFileName() const;

        //! Set new AI queries file name.
        void setAiQueriesFileName(const QString &aiQueriesFileName);

        //! Get const reference to remote file store index file name.
        const QString &getFileStoreIndexFileName() const;

        //! Set new remote file store index file name.
        void setFileStoreIndexFileName(const QString &fileStoreIndexFileName);

};

#endif // AI_QUERY_MANAGER_SETTINGS_H
