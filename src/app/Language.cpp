#include "app/Language.h"

#include <QCoreApplication>
#include <QDebug>

Language &Language::instance()
{
    static Language language;
    return language;
}

void Language::setCode(const QString &code)
{
    if (code == mCode) return;

    QCoreApplication::removeTranslator(&mTranslator);

    if (code == QLatin1String("en")) {
        if (!mTranslator.load(QStringLiteral(":/i18n/sensornode_en.qm"))) {
            qWarning() << "Language: cannot load translation for" << code;
            return;
        }
        QCoreApplication::installTranslator(&mTranslator);
    }

    mCode = code;
    emit changed();
}

void Language::bind(QObject *owner, const std::function<void()> &update)
{
    update();
    connect(&instance(), &Language::changed, owner, update);
}
