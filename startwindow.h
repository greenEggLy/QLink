#pragma once

#ifndef STARTWINDOW_H
#define STARTWINDOW_H

#include <QDialog>

namespace Ui {
class StartWindow;
}

class StartWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StartWindow(QWidget *parent = nullptr);
    ~StartWindow();
    void paintEvent(QPaintEvent*) override;
    void closeEvent(QCloseEvent *) override;

signals:
    void startGame();
    void loadGame();
    void twoPlayers();
    void onePlayer();
    void closeGame();

private slots:
    void on_startButton_clicked();

    void on_loadButton_clicked();

    void on_twoPlayers_stateChanged(int arg1);

private:
    Ui::StartWindow *ui;
};

#endif // STARTWINDOW_H
