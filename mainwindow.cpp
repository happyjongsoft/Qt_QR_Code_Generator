#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);  
    createQRCode();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::requestClose()
{
    emit closeWindow();
}

bool MainWindow::clearQRCode()
{
    QPixmap pix_logo(":/images/qr_logo.png");
    ui->lblLogoImage->setPixmap(pix_logo);

    ui->lblDescription->setText("");

    return true;
}

bool MainWindow::createQRCode()
{
    ui->lblDescription->setText("WELCOME TO JEIPI QR CODE GENERATOR");

    int border = 10;
    int scale = 6;

    QString strQRCode("WELCOME TO JEIPI QR CODE GENERATOR");

    const QrCode qr = QrCode::encodeText(strQRCode.toStdString().c_str(), QrCode::Ecc::LOW);

    QPixmap *pix_qr_code = new QPixmap(qr.getSize() * scale + border * 2, qr.getSize() * scale + border * 2);
    pix_qr_code->fill(Qt::white);
    QPainter *painter_pix_qr_code = new QPainter(pix_qr_code);
    for (int y = 0; y < qr.getSize(); y++) {
        for (int x = 0; x < qr.getSize(); x++) {
            if (qr.getModule(x, y)) {
                painter_pix_qr_code->fillRect(x * scale + border, y * scale + border,
                                              scale, scale, QBrush(Qt::black));
            }
        }
    }

    QPixmap pix_logo(":/images/icon.png");
    pix_logo = pix_logo.scaled(pix_qr_code->width()/5, pix_qr_code->height()/5);

    painter_pix_qr_code->drawPixmap(pix_qr_code->width()/2 - pix_logo.width() / 2, pix_qr_code->height()/2 - pix_logo.height() / 2, pix_logo.width(), pix_logo.height(), pix_logo);

    ui->lblLogoImage->setPixmap(*pix_qr_code);

    return true;
}

