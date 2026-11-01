#ifndef DEVICE_INFO_MODEL_H
#define DEVICE_INFO_MODEL_H

#include <QMetaType>
#include <QObject>
#include <QString>

// Giá trị âm hoặc chuỗi rỗng nghĩa là không đọc được.
struct DeviceInfo
{
    qint64 uptimeSec = -1;
    QString kernel;
    int ramUsedMb = -1;
    int ramTotalMb = -1;
    int dataPercent = -1;
    QString ip;
    bool linkUp = false;
    int linkMbps = -1;
    bool rootReadOnly = false;
    bool verityEnabled = false;
};

Q_DECLARE_METATYPE(DeviceInfo)

class DeviceInfoModel : public QObject
{
    Q_OBJECT

public:
    // rootPath trỏ tới cây thư mục giả khi test, mặc định là / của hệ thống.
    explicit DeviceInfoModel(QObject *parent = nullptr, const QString &rootPath = QString(),
                             const QString &dataMount = QString(), const QString &interfaceName = QString());

    DeviceInfo read() const;

public slots:
    void refresh();

signals:
    void infoChanged(const DeviceInfo &info);

private:
    QString mRoot;
    QString mDataMount;
    QString mInterface;
};

#endif // DEVICE_INFO_MODEL_H
