#ifndef TOUCH_CALIBRATION_H
#define TOUCH_CALIBRATION_H

#include <QByteArray>
#include <QPointF>
#include <QString>
#include <QVector>

// Hệ số module linear của tslib, cùng thứ tự với file pointercal:
// x' = (a2 + a0 * x + a1 * y) / a6, y' = (a5 + a3 * x + a4 * y) / a6.
struct Pointercal
{
    qint64 a[7];
    int xres;
    int yres;
};

// Phép biến đổi affine từ điểm Qt nhận được sang điểm đích trên màn hình.
struct AffineFit
{
    double xx, xy, x0;
    double yx, yy, y0;
};

namespace TouchCalibration {

Pointercal identity(int xres, int yres);
bool parse(const QByteArray &text, Pointercal *out);
QString format(const Pointercal &calibration);

// Bình phương tối thiểu. Trả false nếu các điểm gần như thẳng hàng.
bool solve(const QVector<QPointF> &touched, const QVector<QPointF> &targets, AffineFit *out);

// Qt nhận tọa độ đã qua hiệu chỉnh hiện tại, nên phép sửa phải ghép vào hệ số cũ.
Pointercal compose(const AffineFit &fit, const Pointercal &current);

} // namespace TouchCalibration

#endif // TOUCH_CALIBRATION_H
