#pragma once

#ifndef GAMEAREA_H
#define GAMEAREA_H

#include <QWidget>
#include <vector>
#include <QPushButton>
#include <QTimer>
#include <QKeyEvent>
#include <QDialog>
#include <QLCDNumber>
#include <QDebug>
#include <QMouseEvent>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QPainter>
#include <ctime>
#include <cstdlib>
#include <QString>
#include <QIcon>
#include <QDebug>
#include <QAction>
#include <sstream>
#include "player.h"
#include "closewindow.h"
#include "pausewindow.h"
#include "startwindow.h"

//地图行列数
const int ROW_NUM = 6;
const int COL_NUM = 6;
//设置图标大小
const int IMG_SIZE = 60;
const int PATH_SIZE = 5;
//设置图标种类和每种个数
const int IMG_KIND = 5;
const int EACH_IMG_NUM = 6;
//设置地图起始坐标
const int ST_X = 150;
const int ST_Y = 150;
//设置倒计时
const int LCDTIME = 50;
const int PROTIME = 5;
const int LCD_ST_X = 50;
const int LCD_ST_Y = 50;

QT_BEGIN_NAMESPACE
namespace Ui { class GameArea; }
QT_END_NAMESPACE

struct Point{
    int row,col,kind;
    int x,y;//实际坐标
    int chosen = 0;//是否被选中
    Point(){row = col = kind = x = y = 0;}
    Point(int r,int c,int k,int xx,int yy):row(r),col(c),kind(k),x(xx),y(yy){}
    Point(const Point& p){row = p.row, col = p.col, kind = p.kind,x = p.x,y = p.y;}
};

class Player;

class GameArea : public QWidget
{
    Q_OBJECT
    friend class SimpleTest;

public:
    GameArea(QWidget *parent = nullptr);
    ~GameArea();
    //逻辑地图、实体地图
    std::vector<std::vector<Point>> logicMap;
    std::vector<std::vector<QPushButton*>> buttonMap;
    //get status
    void paintLogicMap();
    int get_row() const;
    int get_col() const;
    int get_each_img_num() const;
    int get_img_kind() const;
    int get_img_size() const;
    int get_path_size() const;
    int get_stx() const;
    int get_sty() const;


private:
    /*------------------地图部分---------------*/
    //地图基础数据
    int rowNum,colNum;
    int imgKind,eachImgNum;
    int imgSize,pathSize;
    int stX,stY;
    //记录每种图标剩余多少
    std::vector<std::vector<Point*>> countImgNumber;
    //玩家可否到达
    std::vector<std::vector<bool>> accessMap1;
    std::vector<std::vector<bool>> accessMap2;
    /*-----------------玩家部分---------------*/
    Player *player1;
    Player *player2;
    QPushButton *player1Img, *player2Img;//图像
    bool hasTwoPlayers = false;
    /*---------------监控部分-----------------*/
    int id =0;
    QTimer *timer1;//刷新
    QTimer *timerPathLink;//连线
    QTimer *timerFlash; //闪现
    QTimer *timerHint;
    QTimer *timerFreeze;
    QTimer *timerDizzy;
    bool canFlash = false;
    bool canHint = false;
    bool ifKeyPressed;
    bool isPaused = false;
    bool startLoadGame = false;
    int keyPressKind;
    int lcdTime;
    int minusTime = 0;
    int propertyKind = 0;
    QPushButton* sol1 = nullptr;
    QPushButton* sol2 = nullptr;
    /*----------------操作部分---------------*/
    QPushButton* prevBlock; //记录上次激活
    std::vector<Point> linkPahtPoints;
    std::vector<QPushButton*> linkButtons;
    /*---------------窗口/控件-------------------*/
    CloseWindow* closeWindow;
    PauseWindow* pauseWindow;
    StartWindow* startWindow;


private:
    //全局
    void initStartWindow();
    void InitParams();
    void InitMaps();
    void InitConnections();
    void disConnections();
    void paintEvent(QPaintEvent* event) override;
    //控制玩家行走
    void keyPressEvent(QKeyEvent *event) override;
    void judgeKey();
    //绘制玩家
    void initPaintPlayer();
    void paintPlayer();
    //进行坐标->行列转换
    int xToRow(QPushButton* btn);
    int yToCol(QPushButton* btn);
    int xToRow(int x);
    int yToCol(int y);
    int colToY(int col) ;
    int rowToX(int row) ;
    //创建逻辑地图并画出
    void createMap();
    void testCreateMap();
    void paintMap();
    void paintPlayingMap();
    void paintHint();
    //设置图标格式
    void paintBlockUncrashed(QPushButton* btn);
    void paintBlockCrashed(QPushButton* btn,int type);
    //判断是否有解、判断是否相连
    bool canConnect(QPushButton* cur,QPushButton* prev);
    bool canConnect(int row1, int col1, int row2, int col2);
    bool connectOneLine(int row1, int col1, int row2, int col2);
    bool connectTwoLines(int row1, int col1, int row2, int col2);
    bool connectThreeLines(int row1, int col1, int row2, int col2);
    //判断是否有全局解
    bool canSolve(QPushButton*& sol1, QPushButton*& sol2);
    void accessToPlayer(int row,int col,int type);
    bool playerCanHandle(int row, int col,int type);
    //绘制连线
    void paintLink(int row1, int col1, int row2, int col2);
    void paintOneLink(int row1, int col1, int row2, int col2);
    //道具
    void addProperty(int propertyKind);
    void judgeProperty(int row,int col);
    void judgeProperty(int row1, int col1, int row2, int col2);
    void shuffle();
    //处理存档信息
    void clearInfo();
    void judgeLoad(QString text);


signals:
    void gameIsOver();
    void flashed();

private slots:
    void Init();
    //绘制选中图标并进行逻辑判断
    void blockCrashed1(int row,int col);
    void blockCrashed2(int row, int col);
    void blockCrashed(int row,int col,int type);
    //消除连线
    void erasePathLink();
    //显示结束窗口
    void showCloseWindow();
    //处理倒计时
    void timerEvent(QTimerEvent*) override;
    //道具
    void cannotFlash();
    void buttonClicked(int, int);
    void on_pauseButton_clicked();
    //存读档
    void saveGame();
    void loadGame();

public:
    Ui::GameArea *ui;
};
#endif // GAMEAREA_H
