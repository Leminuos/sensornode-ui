#ifndef OTA_SETTINGS_H
#define OTA_SETTINGS_H

#include <QJsonObject>
#include <QString>

// Tuỳ chọn OTA phía UI, lưu tại /data/config/ota.json.
class OtaSettings
{
public:
    explicit OtaSettings(const QString &path = QString());

    void load();
    bool save() const;

    // Tự bật popup khi otad báo có bản mới.
    bool autoMode() const;
    void setAutoMode(bool autoMode);

private:
    QString mPath;
    QJsonObject mRoot;
};

#endif // OTA_SETTINGS_H
