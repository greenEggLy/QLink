#include "player.h"
#include "gamearea.h"

Player::Player(QWidget *parent) : QWidget(parent)
{
    std::srand(std::time(nullptr));
    prevBlock = nullptr;
    score = 0;
}

void Player::randPlayerPosition(GameArea* g)
{
    int m = std::rand()%4;
    int n = std::rand()%(g->get_row()+1);
    switch (m) {
    case 0:
        row = 0;
        col = n;
        break;
    case 1:
        row = n;
        col = g->get_col()+1;
        break;
    case 2:
        row = g->get_col()+1;
        col = n+1;
        break;
    case 3:
        row = n+1;
        col = 0;
        break;
    }
    x = g->get_img_size() * row + g->get_stx();
    y = g->get_img_size() * col + g->get_sty();
}

bool Player::judgeCrashBlock(int n_row,int n_col,GameArea* g)
{
    if(g->logicMap[n_row][n_col].kind > 0){
//        g->paintLogicMap();
//        qDebug()<<QString("crash:")<<n_row<<n_col;
        emit crashBlock(n_row,n_col);
        return false;
    }
    return true;
}




//废
void Player::setPlayer(){
    QPushButton player;
    playerImg->setGeometry(x,y,IMG_SIZE,IMG_SIZE);
    playerImg->setIcon(QIcon(":/image/9.jpg"));
    playerImg->setIconSize(QSize(IMG_SIZE,IMG_SIZE));
    playerImg->show();
}

