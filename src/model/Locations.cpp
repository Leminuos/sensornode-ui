#include "model/Locations.h"

#include "app/Language.h"

#include <QDateTime>
#include <QTimeZone>

#include <cstdlib>
#include <ctime>

const QVector<Location> &Locations::all()
{
    static const QVector<Location> locations = {
        {QStringLiteral("Hà Nội"), QStringLiteral("Hanoi"), QStringLiteral("Asia/Ho_Chi_Minh"), 420},
        {QStringLiteral("TP. Hồ Chí Minh"), QStringLiteral("Ho Chi Minh City"), QStringLiteral("Asia/Ho_Chi_Minh"), 420},
        {QStringLiteral("Bangkok"), QStringLiteral("Bangkok"), QStringLiteral("Asia/Bangkok"), 420},
        {QStringLiteral("Singapore"), QStringLiteral("Singapore"), QStringLiteral("Asia/Singapore"), 480},
        {QStringLiteral("Tokyo"), QStringLiteral("Tokyo"), QStringLiteral("Asia/Tokyo"), 540},
        {QStringLiteral("Seoul"), QStringLiteral("Seoul"), QStringLiteral("Asia/Seoul"), 540},
        {QStringLiteral("Sydney"), QStringLiteral("Sydney"), QStringLiteral("Australia/Sydney"), 600},
        {QStringLiteral("Dubai"), QStringLiteral("Dubai"), QStringLiteral("Asia/Dubai"), 240},
        {QStringLiteral("London"), QStringLiteral("London"), QStringLiteral("Europe/London"), 0},
        {QStringLiteral("Paris"), QStringLiteral("Paris"), QStringLiteral("Europe/Paris"), 60},
        {QStringLiteral("New York"), QStringLiteral("New York"), QStringLiteral("America/New_York"), -300},
        {QStringLiteral("Los Angeles"), QStringLiteral("Los Angeles"), QStringLiteral("America/Los_Angeles"), -480},
        {QStringLiteral("UTC"), QStringLiteral("UTC"), QStringLiteral("Etc/UTC"), 0},
    };
    return locations;
}

int Locations::indexOf(const QString &id)
{
    const QVector<Location> &locations = all();
    for (int i = 0; i < locations.size(); ++i) {
        if (locations.at(i).nameEn == id) return i;
    }
    return -1;
}

QString Locations::displayName(const Location &location)
{
    return Language::instance().code() == QLatin1String("en") ? location.nameEn : location.name;
}

static bool hasZoneData(const Location &location)
{
    return QTimeZone(location.timeZone.toUtf8()).isValid();
}

int Locations::offsetMinutes(const Location &location)
{
    if (!hasZoneData(location)) {
        return location.fallbackOffsetMinutes;
    }
    return QTimeZone(location.timeZone.toUtf8()).offsetFromUtc(QDateTime::currentDateTimeUtc()) / 60;
}

QString Locations::offsetText(const Location &location)
{
    const int offset = offsetMinutes(location);
    const int absolute = qAbs(offset);
    QString text = QStringLiteral("UTC%1%2").arg(offset < 0 ? '-' : '+').arg(absolute / 60);
    if (absolute % 60 != 0) {
        text += QStringLiteral(":%1").arg(absolute % 60, 2, 10, QLatin1Char('0'));
    }
    return text;
}

QString Locations::timeText(const Location &location)
{
    return QDateTime::currentDateTimeUtc().addSecs(offsetMinutes(location) * 60).toString(QStringLiteral("HH:mm"));
}

void Locations::apply(const Location &location)
{
    QByteArray value = location.timeZone.toUtf8();
    if (!hasZoneData(location)) {
        // Chuỗi POSIX không cần tzdata. Dấu ngược với UTC+x theo chuẩn POSIX.
        const int offset = location.fallbackOffsetMinutes;
        value = QStringLiteral("<%1%2>%3%4")
                    .arg(offset < 0 ? '-' : '+')
                    .arg(qAbs(offset) / 60, 2, 10, QLatin1Char('0'))
                    .arg(offset < 0 ? '+' : '-')
                    .arg(qAbs(offset) / 60)
                    .toLatin1();
    }
    qputenv("TZ", value);
    tzset();
}
