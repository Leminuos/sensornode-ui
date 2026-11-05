#ifndef LOCATIONS_H
#define LOCATIONS_H

#include <QString>
#include <QVector>

struct Location
{
    QString name;
    QString nameEn;
    QString timeZone;
    int fallbackOffsetMinutes;  // chỉ dùng khi image chưa có tzdata
};

namespace Locations {

const QVector<Location> &all();

// Id lưu trong settingsd là nameEn. Trả về -1 nếu không có.
int indexOf(const QString &id);

QString displayName(const Location &location);

int offsetMinutes(const Location &location);
QString offsetText(const Location &location);
QString timeText(const Location &location);

// Đổi TZ của tiến trình UI để đồng hồ và trục biểu đồ cùng đổi theo.
void apply(const Location &location);

} // namespace Locations

#endif // LOCATIONS_H
