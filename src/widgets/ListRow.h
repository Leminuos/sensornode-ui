#ifndef LIST_ROW_H
#define LIST_ROW_H

#include <QAbstractButton>

class ListRow : public QAbstractButton
{
    Q_OBJECT

public:
    enum Trailing { NoTrailing, Chevron, CheckMark };

    explicit ListRow(const QString &title, QWidget *parent = nullptr);

    // tile là tên màu ô icon: default, yellow, aqua, red, blue, green.
    void setLeadingIcon(const QString &glyph, const QString &tile);
    void setTitle(const QString &title);
    void setSubtitle(const QString &subtitle);
    void setValue(const QString &value);
    void setTrailing(Trailing trailing);
    void setSelected(bool selected);
    void setClockText(const QString &text);
    void setSeparator(bool visible);
    void setPressable(bool pressable);

    QString title() const { return text(); }
    QString value() const { return mValue; }
    bool isSelected() const { return mSelected; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString mGlyph;
    QString mTile;
    QString mSubtitle;
    QString mValue;
    QString mClock;
    Trailing mTrailing = Chevron;
    bool mSelected = false;
    bool mSeparator = true;
    bool mPressable = true;
};

#endif // LIST_ROW_H
