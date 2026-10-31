#include "app/Theme.h"

#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTextStream>

QString Theme::load()
{
    static const QStringList sheets = {
        ":/styles/dashboard.qss", ":/styles/settings.qss",
        ":/styles/firmware_popup.qss", ":/styles/firmware_progress.qss",
        ":/styles/chart.qss", ":/styles/bottomnav.qss",
    };

    QString css;
    for (const QString &path : sheets) {
        QFile file(path);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            qWarning() << "Theme: cannot open" << path;
            continue;
        }
        css += QTextStream(&file).readAll();
        css += '\n';
    }
    return css;
}
