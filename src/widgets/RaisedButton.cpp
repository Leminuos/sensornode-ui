#include "widgets/RaisedButton.h"

#include "app/Theme.h"

#include <QVariant>

RaisedButton::RaisedButton(const QString &text, Variant variant, QWidget *parent)
    : QPushButton(text, parent)
{
    setFocusPolicy(Qt::NoFocus);
    setVariant(variant);
}

void RaisedButton::setVariant(Variant variant)
{
    setProperty("variant", variant == Primary   ? "primary"
                           : variant == Danger ? "danger"
                                               : "secondary");
    Theme::repolish(this);
}

void RaisedButton::setVisualGeometry(const QRect &rect)
{
    setGeometry(rect.marginsAdded(Theme::shadowMargins()));
}

bool RaisedButton::hitButton(const QPoint &pos) const
{
    return rect().marginsRemoved(Theme::shadowMargins()).contains(pos);
}
