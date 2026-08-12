#ifndef AI_QUERY_MANAGER_H
#define AI_QUERY_MANAGER_H

#include <QByteArray>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QThread>
#include <QUuid>

#include <memory>
#include <optional>

#include <rai_agent.h>
#include <rai_agent_file_store.h>
#include <rai_agent_settings.h>
#include <rcl_cloud_ai_query_request.h>
#include <rcl_cloud_ai_query_response.h>

#include "ai_query_info.h"
#include "ai_query_manager_settings.h"
#include "file_manager.h"
#include "service_statistics.h"

class AIQueryManager : public QObject
{

    Q_OBJECT

    protected:

        //! In-flight query: the agent and the worker thread it runs on.
        struct PendingQuery
        {
            RAgent *agent = nullptr;
            QThread *thread = nullptr;
            RCloudAIQueryRequest request;
        };

        //! Completed query result awaiting pickup by its submitter.
        struct CompletedQuery
        {
            RCloudAIQueryResponse response;
            QDateTime completedAt;
        };

        //! File to be uploaded to the AI provider file store and attached to the query.
        struct RemoteFileAttachment
        {
            //! Stable identifier of the source file (cloud file-store id).
            QString storeKey;
            //! File name reported to the provider.
            QString fileName;
            //! File content snapshot.
            QByteArray content;
            //! Detected content mime type.
            QString mimeType;
        };

        //! How long a completed result is retained before it is discarded unfetched.
        static const qint64 resultRetentionSecs = 3600;

        //! Settings.
        AIQueryManagerSettings settings;
        //! Service statistics.
        ServiceStatistics statistics;
        //! File manager service (sole gateway to the file store).
        FileManager *fileManager;
        //! List of AI query application profiles.
        QList<AIQueryInfo> aiQueries;
        //! System prompt for unknown or unspecified applications.
        QString defaultSystemPrompt;
        //! Preamble introducing an attached file in the agent prompt.
        QString filePromptPreamble;
        //! Map of in-flight queries by request id.
        QMap<QUuid, PendingQuery> pendingQueries;
        //! Map of completed results by request id, kept until fetched or expired.
        QMap<QUuid, CompletedQuery> completedResults;
        //! Map of file manager request ids to AI query request ids.
        QMap<QUuid, QUuid> fileRequests;
        //! Remote (Anthropic Files API) file store; null for other agent types.
        std::unique_ptr<RAgentFileStore> agentFileStore;

    public:

        //! Constructor.
        explicit AIQueryManager(const AIQueryManagerSettings &settings, FileManager *fileManager, QObject *parent = nullptr);

        //! Return const reference to list of AI query application profiles.
        const QList<AIQueryInfo> &getAIQueries() const;

        //! Read from file.
        void readFile();

        //! Write to file.
        void writeFile() const;

        //! Submit AI query. Returns the request id used to fetch the result.
        QUuid submitQuery(const RCloudAIQueryRequest &aiQueryRequest);

        //! Fetch the result of a query by its request id. Returns a pending
        //! response while the query is in flight and the completed response
        //! (removed from retention) once it settles. Only the submitting user
        //! (or root) may fetch; an unknown or expired id is an error.
        RCloudAIQueryResponse fetchQueryResult(const QUuid &requestId, const RUserInfo &executor);

        //! Get statistics output in Json form.
        QJsonObject getStatisticsJson() const;

    private:

        //! Build agent settings for a given request (model may be overridden per request).
        RAgentSettings buildAgentSettings(const RCloudAIQueryRequest &aiQueryRequest) const;

        //! Return the system prompt (context and guardrails) for an application.
        //! Unknown or unspecified applications get the default system prompt.
        QString systemPromptForApplication(const QString &application) const;

        //! Start the agent on its worker thread for a pending query. When an
        //! attachment is given it is first uploaded to the remote file store
        //! (on the worker thread) and referenced from the message.
        void startAgent(const QUuid &requestId, const QString &prompt, const std::optional<RemoteFileAttachment> &attachment = std::nullopt);

        //! Build the full agent prompt for a query with an attached file:
        //! preamble, fenced file block (metadata and content), then the query.
        QString buildFilePrompt(const RCloudAIQueryRequest &request, const FileObject &object, const QString &fileContent) const;

        //! Build the agent prompt for a query whose file is attached through
        //! the provider file store: preamble, metadata only, then the query.
        QString buildRemoteFilePrompt(const RCloudAIQueryRequest &request, const FileObject &object) const;

        //! Detect the mime type of content accepted by the provider file store
        //! (PDF, PNG, JPEG, GIF, WebP or UTF-8 text). Returns an empty string
        //! for unsupported content.
        static QString detectRemoteMimeType(const QByteArray &content);

        //! Finalize a query: move it from the pending map to the completed
        //! results retention and emit completion.
        void finishQuery(const QUuid &requestId, const QString &responseText);

        //! Discard completed results that were never fetched within the retention period.
        void purgeExpiredResults();

        //! Create AI query manager object from Json.
        void fromJson(const QJsonObject &json);

        //! Create Json from AI query manager object.
        QJsonObject toJson() const;

    private slots:

        //! File manager request is completed (delivers the file snapshot for a query).
        void onFileRequestCompleted(const QUuid &requestId, QSharedPointer<const FileObject> object);

    signals:

        //! AI query completed.
        void queryCompleted(const QUuid &requestId, const RCloudAIQueryResponse &response);

};

#endif // AI_QUERY_MANAGER_H
