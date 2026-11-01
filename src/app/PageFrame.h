#ifndef PAGE_FRAME_H
#define PAGE_FRAME_H

#include <QHash>
#include <QVector>
#include <QWidget>

class BottomNavBar;
class QStackedWidget;
class StatusHeader;
class SubPage;

class PageFrame : public QWidget
{
    Q_OBJECT

public:
    enum class Mode {
        Tab,        // status bar, nội dung 246 px, thanh tab
        SubPage,    // status bar, trang con 294 px
        Takeover,   // status bar, trang 294 px không có đường quay lại, dùng cho OTA
        FullScreen  // không có status bar
    };

    explicit PageFrame(QWidget *parent = nullptr);

    StatusHeader *statusHeader() const { return mStatusHeader; }
    BottomNavBar *navBar() const { return mNavBar; }
    QStackedWidget *stack() const { return mStack; }
    Mode mode() const { return mMode; }
    QWidget *currentPage() const;

    void addTab(int tab, QWidget *page);
    void showTab(int tab);

    void addPage(SubPage *page);
    void pushPage(SubPage *page);
    void popPage();

    void showTakeover(QWidget *page);
    void showFullScreenPage(QWidget *page);

private:
    void setMode(Mode mode);
    void showPage(QWidget *page);

    StatusHeader *mStatusHeader;
    QStackedWidget *mStack;
    BottomNavBar *mNavBar;

    Mode mMode = Mode::Tab;
    int mCurrentTab = 0;
    QHash<int, QWidget *> mTabs;
    QVector<SubPage *> mPageTrail;
};

#endif // PAGE_FRAME_H
