#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QJsonObject>
#include <QString>

class AppSettings
{
public:
    explicit AppSettings(const QString &path = QString());

    void load();
    bool save() const;

    int brightness() const;
    void setBrightness(int percent);

private:
    QString mPath;
    QJsonObject mRoot;
};

#endif // APP_SETTINGS_H
