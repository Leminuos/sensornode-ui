#include "app/Theme.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

// Resource nằm trong thư viện tĩnh nên phải kéo vào tường minh, và macro không dùng được trong namespace.
static void initResources()
{
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(assets);
}

namespace {

struct Tokens
{
    QHash<QString, QString> colors;
    QHash<QString, int> metrics;
};

const Tokens &tokens()
{
    static const Tokens instance = []() {
        initResources();

        Tokens t;
        QFile file(QStringLiteral(":/theme/tokens.json"));
        if (!file.open(QFile::ReadOnly)) {
            qWarning() << "Theme: cannot open" << file.fileName();
            return t;
        }

        const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
        const QJsonObject colors = root.value(QStringLiteral("color")).toObject();
        for (auto it = colors.begin(); it != colors.end(); ++it) {
            t.colors.insert(it.key(), it.value().toString());
        }

        for (const char *group : {"font", "radius", "space", "size"}) {
            const QString prefix = QLatin1String(group);
            const QJsonObject values = root.value(prefix).toObject();
            for (auto it = values.begin(); it != values.end(); ++it) {
                t.metrics.insert(prefix + '-' + it.key(), it.value().toInt());
            }
        }
        return t;
    }();
    return instance;
}

QString resolve(const QString &css)
{
    static const QRegularExpression token(QStringLiteral("@([a-z][a-z0-9-]*)"));

    QString out;
    out.reserve(css.size());
    int last = 0;
    auto it = token.globalMatch(css);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const QString name = match.captured(1);
        out += css.midRef(last, match.capturedStart() - last);
        last = match.capturedEnd();

        if (tokens().colors.contains(name)) {
            out += tokens().colors.value(name);
        } else if (tokens().metrics.contains(name)) {
            out += QString::number(tokens().metrics.value(name)) + QLatin1String("px");
        } else {
            qWarning() << "Theme: unknown token" << name;
        }
    }
    out += css.midRef(last);
    return out;
}

} // namespace

QString Theme::load()
{
    static const QStringList sheets = {
        ":/styles/dashboard.qss", ":/styles/settings.qss",
        ":/styles/firmware_popup.qss", ":/styles/firmware_progress.qss",
        ":/styles/chart.qss", ":/styles/theme.qss",
    };

    tokens();

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
    return resolve(css);
}

void Theme::apply(QApplication &app)
{
    QFont font(QStringLiteral("DejaVu Sans"));
    font.setPixelSize(metric(QStringLiteral("font-body")));
    app.setFont(font);
    app.setStyleSheet(load());
}

QColor Theme::color(const QString &name)
{
    const auto it = tokens().colors.constFind(name);
    if (it == tokens().colors.constEnd()) {
        qWarning() << "Theme: unknown color" << name;
        return QColor();
    }
    return QColor(*it);
}

int Theme::metric(const QString &name)
{
    const auto it = tokens().metrics.constFind(name);
    if (it == tokens().metrics.constEnd()) {
        qWarning() << "Theme: unknown metric" << name;
        return 0;
    }
    return *it;
}

QFont Theme::font(const QString &size, bool bold)
{
    QFont font(QStringLiteral("DejaVu Sans"));
    font.setPixelSize(metric(QStringLiteral("font-") + size));
    font.setBold(bold);
    return font;
}

QPixmap Theme::icon(const QString &name, int size, const QString &colorName)
{
    static QHash<QString, QPixmap> cache;

    const QString key = QStringLiteral("%1_%2_%3").arg(name).arg(size).arg(colorName);
    const auto cached = cache.constFind(key);
    if (cached != cache.constEnd()) {
        return *cached;
    }

    tokens();

    QPixmap tinted(QStringLiteral(":/icons/%1_%2.png").arg(name).arg(size));
    if (tinted.isNull()) {
        qWarning() << "Theme: unknown icon" << name << size;
        return tinted;
    }

    QPainter painter(&tinted);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(tinted.rect(), color(colorName));
    painter.end();

    cache.insert(key, tinted);
    return tinted;
}

QMargins Theme::shadowMargins()
{
    return QMargins(metric(QStringLiteral("size-shadow-left")), metric(QStringLiteral("size-shadow-top")),
                    metric(QStringLiteral("size-shadow-right")), metric(QStringLiteral("size-shadow-bottom")));
}
