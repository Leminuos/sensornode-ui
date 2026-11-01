#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QObject>
#include <QString>
#include <QTranslator>

#include <functional>

class Language : public QObject
{
    Q_OBJECT

public:
    static Language &instance();

    QString code() const { return mCode; }
    void setCode(const QString &code);

    static void bind(QObject *owner, const std::function<void()> &update);

signals:
    void changed();

private:
    Language() = default;

    QTranslator mTranslator;
    QString mCode = QStringLiteral("vi");
};

#endif // LANGUAGE_H
