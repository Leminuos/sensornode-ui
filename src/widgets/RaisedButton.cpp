#include "widgets/RaisedButton.h"

#include "app/Theme.h"

#include <QVariant>

RaisedButton::RaisedButton(const QString &text, Variant variant, QWidget *parent)
    : QPushButton(text, parent)
{
    setProperty("variant", variant == Primary   ? "primary"
                           : variant == Danger ? "danger"
                                               : "secondary");
    setFocusPolicy(Qt::NoFocus);
}

void RaisedButton::setVisualGeometry(const QRect &rect)
{
    setGeometry(rect.marginsAdded(Theme::shadowMargins()));
}

bool RaisedButton::hitButton(const QPoint &pos) const
{
    return rect().marginsRemoved(Theme::shadowMargins()).contains(pos);
}
