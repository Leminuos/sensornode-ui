#ifndef CALIBRATION_FILE_H
#define CALIBRATION_FILE_H

#include <QString>

// pointercal của tslib cùng bản .bak và cờ .pending để người dùng xác nhận sau khi UI khởi động lại.
class CalibrationFile
{
public:
    explicit CalibrationFile(const QString &path = QString());

    QString path() const { return mPath; }
    bool isPending() const;

    bool apply(const QString &pointercal);
    void keep();
    bool revert();

private:
    QString mPath;
};

#endif // CALIBRATION_FILE_H
