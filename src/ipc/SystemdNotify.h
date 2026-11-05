#ifndef SYSTEMD_NOTIFY_H
#define SYSTEMD_NOTIFY_H

#include <QByteArray>

namespace SystemdNotify {

// Gửi tới NOTIFY_SOCKET, không làm gì nếu không chạy dưới systemd.
void send(const QByteArray &state);

} // namespace SystemdNotify

#endif // SYSTEMD_NOTIFY_H
