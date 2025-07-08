#include <QDebug>
#include <QSocketNotifier>

#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

#include <rbl_logger.h>

#include "unix_signal_handler.h"

std::array<QPointer<UnixSignalHandler>, UnixSignalHandler::max_signal> UnixSignalHandler::handler;

UnixSignalHandler::UnixSignalHandler(int signal, QObject *parent)
    : QObject(parent)
{
    if (UnixSignalHandler::handler[signal] != nullptr)
    {
        RLogger::error("Ignoring request to register duplicate handler for signal \'%d\'.",signal);
        return;
    }
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fd))
    {
        RLogger::error("Failed to create socket pair for signal \'%d\'. %s.",signal, strerror(errno));
        return;
    }
    // There's not very much that a signal handler can legally do.  One thing
    // that is permitted is to write to an open file descriptor.  When our
    // handler is called, we'll write a single byte to a socket, and this socket
    // notifier will then learn of the signal outside of the signal handler
    // context.
    auto notifier = new QSocketNotifier(fd[1], QSocketNotifier::Read, this);
    QObject::connect(notifier, &QSocketNotifier::activated, this, &UnixSignalHandler::consumeInput);

    struct sigaction action;
    action.sa_handler = &UnixSignalHandler::handle;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    if (::sigaction(signal, &action, 0))
    {
        RLogger::error("Failed to add sigaction for signal \'%d\'. %s.",signal, strerror(errno));
        return;
    }

    UnixSignalHandler::handler[signal] = this;
}

// This slot is connected to our socket notifier.  It reads the byte that the
// signal handler wrote (to reset the notifier) and emits a Qt signal.
void UnixSignalHandler::consumeInput(int fd)
{
    char c;
    if (::read(fd, &c, sizeof c) <= 0)
    {
        RLogger::warning("Error reading fd \'%d\' (ignored). %s.",fd, strerror(errno));
    }
    emit this->raised();
}

// This static method is the signal handler called when the process receives a
// Unix signal.  It writes a single byte to our open file descriptor.
void UnixSignalHandler::handle(int signal)
{
    if (signal < 0 || static_cast<size_t>(signal) >= UnixSignalHandler::handler.size())
    {
        RLogger::warning("Ignored out-of-range signal \'%d\'.",signal);
        return;
    }

    auto const h = UnixSignalHandler::handler[signal];
    if (!h)
    {
        RLogger::warning("Ignored unhandled signal \'%d\'.",signal);
        return;
    }

    char c = 0;
    if (::write(h->fd[0], &c, sizeof c) <= 0)
    {
        RLogger::warning("Error writing signal \'%d\' (ignored). %s.",signal, strerror(errno));
    }
}
