#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QDateTime>
#include <QTimer>

#include "qrcodegen.hpp"

using namespace qrcodegen;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void requestClose();

private:
    bool        clearQRCode();
    bool        createQRCode();
signals:
    void        closeWindow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
