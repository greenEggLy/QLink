#include "pausewindow.h"
#include "ui_pausewindow.h"

PauseWindow::PauseWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PauseWindow)
{
    ui->setupUi(this);
}

PauseWindow::~PauseWindow()
{
    delete ui;
}

void PauseWindow::on_resumeButton_clicked()
{
    emit resumeGame();
    this->close();
}

void PauseWindow::on_saveButton_clicked()
{
    emit saveGame();
}

void PauseWindow::on_loadButton_clicked()
{
    emit loadGame();
}
