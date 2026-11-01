#include "model/TouchCalibration.h"

#include <QList>
#include <QStringList>
#include <QtMath>

static constexpr qint64 kScale = 65536;
static constexpr double kMinDeterminant = 1e-6;

static double det3(const double m[3][3])
{
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

static void solve3(const double m[3][3], const double right[3], double det, double out[3])
{
    for (int column = 0; column < 3; ++column) {
        double replaced[3][3];
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                replaced[r][c] = c == column ? right[r] : m[r][c];
            }
        }
        out[column] = det3(replaced) / det;
    }
}

Pointercal TouchCalibration::identity(int xres, int yres)
{
    return Pointercal{{kScale, 0, 0, 0, kScale, 0, kScale}, xres, yres};
}

bool TouchCalibration::parse(const QByteArray &text, Pointercal *out)
{
    const QList<QByteArray> fields = text.simplified().split(' ');
    if (fields.size() < 7) return false;

    Pointercal parsed = identity(0, 0);
    for (int i = 0; i < 7; ++i) {
        bool ok = false;
        parsed.a[i] = fields.at(i).toLongLong(&ok);
        if (!ok) return false;
    }
    if (parsed.a[6] == 0) return false;

    if (fields.size() >= 9) {
        parsed.xres = fields.at(7).toInt();
        parsed.yres = fields.at(8).toInt();
    }
    *out = parsed;
    return true;
}

QString TouchCalibration::format(const Pointercal &c)
{
    QStringList fields;
    for (qint64 value : c.a) {
        fields << QString::number(value);
    }
    fields << QString::number(c.xres) << QString::number(c.yres);
    return fields.join(QLatin1Char(' '));
}

bool TouchCalibration::solve(const QVector<QPointF> &touched, const QVector<QPointF> &targets, AffineFit *out)
{
    if (touched.size() != targets.size() || touched.size() < 3) return false;

    double m[3][3] = {};
    double rightX[3] = {};
    double rightY[3] = {};

    for (int i = 0; i < touched.size(); ++i) {
        const double row[3] = {touched.at(i).x(), touched.at(i).y(), 1.0};
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                m[r][c] += row[r] * row[c];
            }
            rightX[r] += row[r] * targets.at(i).x();
            rightY[r] += row[r] * targets.at(i).y();
        }
    }

    const double det = det3(m);
    if (qAbs(det) < kMinDeterminant) return false;

    double x[3];
    double y[3];
    solve3(m, rightX, det, x);
    solve3(m, rightY, det, y);
    *out = AffineFit{x[0], x[1], x[2], y[0], y[1], y[2]};
    return true;
}

Pointercal TouchCalibration::compose(const AffineFit &fit, const Pointercal &current)
{
    const qint64 *a = current.a;

    Pointercal out = current;
    out.a[0] = qRound64(fit.xx * a[0] + fit.xy * a[3]);
    out.a[1] = qRound64(fit.xx * a[1] + fit.xy * a[4]);
    out.a[2] = qRound64(fit.xx * a[2] + fit.xy * a[5] + fit.x0 * a[6]);
    out.a[3] = qRound64(fit.yx * a[0] + fit.yy * a[3]);
    out.a[4] = qRound64(fit.yx * a[1] + fit.yy * a[4]);
    out.a[5] = qRound64(fit.yx * a[2] + fit.yy * a[5] + fit.y0 * a[6]);
    return out;
}
