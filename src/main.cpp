#include "app/AppWindow.h"
#include "app/FirstFrameNotifier.h"
#include "app/Theme.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Theme::apply(app);

    AppWindow window;
    window.show();
    new FirstFrameNotifier(window.windowHandle());

    return app.exec();
}
