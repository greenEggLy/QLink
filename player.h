#pragma once

#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QPushButton>
#include <ctime>
#include <cstdlib>
#include <QDebug>
#include "gamearea.h"

class GameArea;

class Player : public QWidget
{
    Q_OBJECT
    friend class GameArea;
public:
    explicit Player(QWidget *parent = nullptr);
    bool judgeCrashBlock(int n_row, int n_col,GameArea* g);
    QWidget* qWidget;

public:
    int x, y;
    int row,col;
    QPushButton *playerImg;
    QPushButton *prevBlock;
    int score;
    bool freeze = false, dizzy = false;
private:
    void randPlayerPosition(GameArea* g);
    void setPlayer();

signals:
    void crashBlock(int,int);
};

#endif // PLAYER_H
