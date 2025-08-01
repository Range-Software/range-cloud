#ifndef UNIX_SIGNAL_HANDLER_H
#define UNIX_SIGNAL_HANDLER_H

#include <QObject>
#include <QPointer>

#include <array>

// A UnixSignalHandler catches a particular Unix signal (e.g. SIGTERM) and emits
// a Qt signal which can be connected to a slot.  Note that a process cannot
// catch SIGKILL - a handler for SIGKILL will never emit.
class UnixSignalHandler: public QObject
{
    Q_OBJECT

    public:
        UnixSignalHandler(int signal, QObject *parent = nullptr);

        static const int max_signal = 32;

    signals:
        // This gives no indication of which signal has been caught; you may achieve
        // that by connecting to a QSignalMapper if required.
        void raised();

    private slots:
        void consumeInput(int fd);

    private:

        int fd[2];
        static std::array<QPointer<UnixSignalHandler>, max_signal> handler;
        static void handle(int signal);
};

#endif // UNIX_SIGNAL_HANDLER_H
