#include "ipc/SystemdNotify.h"

#include <QtGlobal>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstddef>
#include <cstring>

void SystemdNotify::send(const QByteArray &state)
{
    const QByteArray path = qgetenv("NOTIFY_SOCKET");
    sockaddr_un addr{};
    if (path.isEmpty() || (path[0] != '/' && path[0] != '@')
            || size_t(path.size()) >= sizeof addr.sun_path) {
        return;
    }

    addr.sun_family = AF_UNIX;
    std::memcpy(addr.sun_path, path.constData(), size_t(path.size()));
    if (addr.sun_path[0] == '@') {
        addr.sun_path[0] = '\0';
    }

    const int fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        qWarning("sd_notify socket failed");
        return;
    }

    const auto length = socklen_t(offsetof(sockaddr_un, sun_path) + size_t(path.size()));
    if (sendto(fd, state.constData(), size_t(state.size()), MSG_NOSIGNAL,
               reinterpret_cast<const sockaddr *>(&addr), length) < 0) {
        qWarning("sd_notify sendto failed");
    }
    close(fd);
}
