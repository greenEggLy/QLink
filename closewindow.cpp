#include "closewindow.h"
#include "ui_closewindow.h"

CloseWindow::CloseWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CloseWindow)
{
    ui->setupUi(this);
    connect(this,SIGNAL(exitGame()),this,SLOT(close()));
    ui->youWin->hide();
    ui->youLose->hide();
    ui->player1Win->hide();
    ui->player2Win->hide();
    ui->tie->hide();
    //connect(this,SIGNAL(reStartGame()),this,SLOT(close()));
}


CloseWindow::~CloseWindow()
{
    delete ui;
}

void CloseWindow::judgeState(int s)
{
    switch (s) {
    case 1:
        ui->player1Win->show();
        break;
    case 2:
        ui->player2Win->show();
        break;
    case 3:
        ui->youWin->show();
        break;
    case 4:
        ui->youLose->show();
        break;
    case 5:
        ui->tie->show();
        break;
    }
}

void CloseWindow::on_exitButton_clicked()
{
    emit exitGame();
}

void CloseWindow::on_restartButton_clicked()
{
    this->close();
    emit reStartGame();
}
