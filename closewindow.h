#pragma once

#ifndef CLOSEWINDOW_H
#define CLOSEWINDOW_H

#include <QWidget>
#include <QPushButton>

namespace Ui {
class CloseWindow;
}

class CloseWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CloseWindow(QWidget *parent = nullptr);
    void judgeState(int s);
    ~CloseWindow();

signals:
    void reStartGame();
    void exitGame();

private slots:
    void on_exitButton_clicked();

    void on_restartButton_clicked();

private:
    Ui::CloseWindow *ui;
};

#endif // CLOSEWINDOW_H
