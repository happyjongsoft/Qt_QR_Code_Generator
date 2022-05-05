#include "mainwindow.h"
#include <QApplication>
#include "framelesswindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FramelessWindow framelessWindow;
    MainWindow *mainWindow = new MainWindow;

    framelessWindow.setContent(mainWindow);
    framelessWindow.connect(&framelessWindow, &FramelessWindow::requestClose, mainWindow, &MainWindow::requestClose);
    mainWindow->connect(mainWindow, &MainWindow::closeWindow, &framelessWindow, &QWidget::close);
    framelessWindow.show();

    return a.exec();
}
