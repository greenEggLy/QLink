#include "startwindow.h"
#include "ui_startwindow.h"
#include <QPixmap>
#include <QPalette>
#include <QPainter>
StartWindow::StartWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartWindow)
{
    ui->setupUi(this);
//   setStyleSheet("background-image:url(:/image/0.png)");
//    QPixmap  pixmap = QPixmap(":/img/0.png").scaled(this->size());
//    QPalette palette;
//    palette.setBrush(QPalette::Window,QBrush(pixmap));
    setWindowTitle(QString("Welcome to Link Game!"));
    setFixedSize(QSize(450,450));
}

StartWindow::~StartWindow()
{
    delete ui;
}

void StartWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QPixmap pixmap(":/image/0.png");
    painter.drawPixmap(this->rect(),pixmap);
}

void StartWindow::on_startButton_clicked()
{
    emit startGame();
}

void StartWindow::on_loadButton_clicked()
{
    this->close();
    emit loadGame();
}


void StartWindow::on_twoPlayers_stateChanged(int arg1)
{
    if(arg1) emit twoPlayers();
    else emit onePlayer();
}

void StartWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit closeGame();
}
