#ifndef LANGUAGE_LOCATION_PAGE_H
#define LANGUAGE_LOCATION_PAGE_H

#include "widgets/SubPage.h"

class ListRow;
class SegmentedControl;

class LanguageLocationPage : public SubPage
{
    Q_OBJECT

public:
    explicit LanguageLocationPage(QWidget *parent = nullptr);

public slots:
    void setLanguage(const QString &language);
    void setLocationName(const QString &name);

signals:
    void languageRequested(const QString &language);
    void locationRequested();

private:
    SegmentedControl *mLanguage;
    ListRow *mLocation;
};

#endif // LANGUAGE_LOCATION_PAGE_H
