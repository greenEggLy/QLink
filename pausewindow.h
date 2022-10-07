#pragma once

#ifndef PAUSEWINDOW_H
#define PAUSEWINDOW_H

#include <QDialog>

namespace Ui {
class PauseWindow;
}

class PauseWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PauseWindow(QWidget *parent = nullptr);
    ~PauseWindow();

private slots:
    void on_resumeButton_clicked();

    void on_saveButton_clicked();

    void on_loadButton_clicked();

private:
    Ui::PauseWindow *ui;

signals:
    void resumeGame();
    void saveGame();
    void loadGame();
};

#endif // PAUSEWINDOW_H
