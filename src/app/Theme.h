#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QFont>
#include <QMargins>
#include <QPixmap>
#include <QString>

class QApplication;
class QWidget;

namespace Theme {

QString load();
void apply(QApplication &app);

QColor color(const QString &name);
int metric(const QString &name);
QFont font(const QString &size, bool bold = false);

// Icon là mặt nạ alpha, tô theo token lúc nạp.
QPixmap icon(const QString &name, int size, const QString &colorName);

void repolish(QWidget *widget);

// Phần trong suốt quanh bề mặt nổi, chứa bóng đã bake.
QMargins shadowMargins();

} // namespace Theme

#endif // THEME_H
