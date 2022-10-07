#include "gamearea.h"
#include "ui_gamearea.h"


GameArea::GameArea(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameArea)
{
    ui->setupUi(this);
    //global
    this->setFocusPolicy(Qt::StrongFocus);
    setWindowTitle(QString("Link-Game"));
    setFixedSize(QSize(800,800));
    // this->setMouseTracking(true);
    initStartWindow();
    Init();
}

//初始化设置
void GameArea::Init()
{
    std::srand(std::time(nullptr));
    InitParams();
    InitMaps();
    InitConnections();
    if(startLoadGame) loadGame();
    qDebug("init success");
}
void GameArea::initStartWindow()
{
    startWindow = new StartWindow;
    connect(startWindow,&StartWindow::startGame,startWindow,&StartWindow::close);
    connect(startWindow,&StartWindow::loadGame,this,[=]()mutable{startLoadGame = true;startWindow->close();});
    connect(startWindow,&StartWindow::twoPlayers,this,[=]()mutable{hasTwoPlayers = true;});
    connect(startWindow,&StartWindow::onePlayer,this,[=]()mutable{hasTwoPlayers = false;});
    connect(startWindow,&StartWindow::closeGame,this,&GameArea::close);
    startWindow->exec();
}
void GameArea::InitParams()
{
    prevBlock = nullptr;
    closeWindow = new CloseWindow;
    pauseWindow = new PauseWindow;
    logicMap.clear();
    linkPahtPoints.clear();
    linkButtons.clear();
    accessMap1.clear();
    accessMap2.clear();
    countImgNumber.clear();
    //map
    rowNum = ROW_NUM; colNum = COL_NUM;
    eachImgNum = EACH_IMG_NUM;
    imgKind = IMG_KIND;
    imgSize = IMG_SIZE; pathSize = PATH_SIZE;
    stX = ST_X; stY = ST_Y;
    minusTime = 0;
    lcdTime = LCDTIME;
    //player
    player1 = nullptr;
    player2 = nullptr;
//    hasTwoPlayers = true;
    player1 = new Player;
    if(hasTwoPlayers) player2 = new Player;
    ifKeyPressed = false;
    keyPressKind = 0;
    player1Img = new QPushButton(this);
    player2Img = new QPushButton(this);
    //timer
    timer1 = new QTimer(this);
    timer1 ->start(10);
    lcdTime = LCDTIME;
    timerFlash = new QTimer(this);
    timerHint = new QTimer(this);
    timerFreeze = new QTimer(this);
    timerDizzy = new QTimer(this);
    timerFlash->setSingleShot(true);
    timerHint->setSingleShot(true);
    timerFreeze->setSingleShot(true);
    timerDizzy->setSingleShot(true);
    ui->lcdNumber->setDigitCount(3);
    ui->lcdNumber->display(QString::number(lcdTime));
    ui->scoreOne->setDigitCount(4);
    ui->scoreTwo->setDigitCount(4);
    ui->scoreOne->display(QString::number(0));
    ui->scoreTwo->display(QString::number(0));
    if(!hasTwoPlayers){
        ui->scoreTwo->hide();
        ui->label_2->hide();
    }
    id = startTimer(1000);
}
void GameArea::InitMaps()
{
    createMap();
    //when proceeding the test, use testCreateMap()
    //testCreateMap();
    paintMap();
    initPaintPlayer();
    paintLogicMap();
}

void GameArea::InitConnections()
{
    int i,j;
    connect(timer1,SIGNAL(timeout()),this,SLOT(update()));
    connect(timerFlash,&QTimer::timeout,timerFlash,&QTimer::stop);
    connect(timerFlash,&QTimer::timeout,this,[=]()mutable{canFlash=false;});
    connect(timerHint,&QTimer::timeout,timerHint,&QTimer::stop);
    connect(timerHint,&QTimer::timeout,this,[=]()mutable{qDebug("cannotHint");canHint = false;});
    connect(timerFreeze,&QTimer::timeout,timerFreeze,&QTimer::stop);
    connect(timerFreeze,&QTimer::timeout,this,[=]()mutable{player1->freeze = false;if(hasTwoPlayers)player2->freeze = false;});
    connect(timerDizzy,&QTimer::timeout,timerDizzy,&QTimer::stop);
    connect(timerDizzy,&QTimer::timeout,this,[=]()mutable{player1->dizzy = false;if(hasTwoPlayers)player2->dizzy = false;});
    connect(player1,SIGNAL(crashBlock(int,int)),this,SLOT(blockCrashed1(int,int)));
    if(hasTwoPlayers){
        connect(player2,SIGNAL(crashBlock(int,int)),this,SLOT(blockCrashed2(int,int)));
    }
    connect(this, SIGNAL(gameIsOver()),this,SLOT(showCloseWindow()));
    connect(closeWindow,SIGNAL(exitGame()),this,SLOT(close()));
    connect(closeWindow,&CloseWindow::reStartGame,this,[=]()mutable{clearInfo();delete player1Img;delete player2Img;Init();});
    connect(pauseWindow,&PauseWindow::resumeGame,this,[=]()mutable{isPaused = false;});
    connect(pauseWindow,&PauseWindow::saveGame,this,[=]()mutable{
        pauseWindow->close(); isPaused = true;
        saveGame();
    });
    connect(pauseWindow,&PauseWindow::loadGame,this,[=]()mutable{
       pauseWindow->close(); isPaused = true;
       loadGame();
    });
    for(i = 0; i <= rowNum+1; ++i){
        for(j = 0; j <= colNum+1; ++j){
            QPushButton* btn = buttonMap[i][j];
            connect(btn,&QPushButton::clicked,this,[=](){buttonClicked(i,j);});
        }
    }
}

void GameArea::disConnections()
{
    disconnect(pauseWindow,SIGNAL(saveGame()),nullptr,nullptr);
    disconnect(pauseWindow,SIGNAL(loadGame()),nullptr,nullptr);
    disconnect(pauseWindow,nullptr,this,nullptr);
}

//刷新
void GameArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QPixmap pixmap(":/image/0.png");
    painter.drawPixmap(this->rect(),pixmap);
    ui->scoreOne->display(QString::number(player1->score));
    if(hasTwoPlayers) ui->scoreTwo->display(QString::number(player2->score));
    if(canHint) canSolve(sol1,sol2);
    paintPlayingMap();
    paintPlayer();
}

//创建逻辑地图，包含图标种类，实际坐标
void GameArea::createMap()
{
    logicMap.clear();
    countImgNumber.clear();
    //创建地图，四周为空，设置实际坐标
    for(int i = 0; i <= rowNum+1; ++i){
        std::vector<Point> tmp;
        std::vector<bool> tmpp;
        std::vector<bool> tmpp2;
        for(int j = 0; j <= colNum+1; ++j){
            tmp.push_back(Point(i,j,0,stX + i*imgSize,stY + j*imgSize));
            tmpp.push_back(false);
            tmpp2.push_back(false);
//            tmp.back().x = stX + i*imgSize;
//            tmp.back().y = stY + j*imgSize;
        }
        logicMap.push_back(tmp);
        accessMap1.push_back(tmpp);
        accessMap2.push_back(tmpp2);
    }
    //每种图标分别放入
    std::vector<Point*> zero;
    countImgNumber.push_back(zero);//零号位，不计入
    for(int i = 1; i <= imgKind; ++i){
        std::vector<Point*> tmp;
        for(int j = 1; j <= eachImgNum; ++j){
            Point* img = new Point(0,0,0,0,0);
            while(true){
                int r = std::rand()%rowNum + 1;
                int c = std::rand()%colNum + 1;
                if(!logicMap[r][c].kind){
                    logicMap[r][c].kind = i;
                    img->row = r, img->col = c;
                    img->kind = i;
                    img->x = rowToX(r), img->y = colToY(c);
                    tmp.push_back(img);
                    break;
                }
            }
        }
        countImgNumber.push_back(tmp);
    }
}

void GameArea::testCreateMap()
{
    logicMap.clear();
    countImgNumber.clear();
    //创建地图，四周为空，设置实际坐标
    for(int i = 0; i <= rowNum+1; ++i){
        std::vector<Point> tmp;
        std::vector<bool> tmpp;
        std::vector<bool> tmpp2;
        for(int j = 0; j <= colNum+1; ++j){
            tmp.push_back(Point(i,j,0,stX + i*imgSize,stY + j*imgSize));
            tmpp.push_back(false);
            tmpp2.push_back(false);
//            tmp.back().x = stX + i*imgSize;
//            tmp.back().y = stY + j*imgSize;
        }
        logicMap.push_back(tmp);
        accessMap1.push_back(tmpp);
        accessMap2.push_back(tmpp2);
    }
    //每种图标分别放入
    std::vector<Point*> zero;
    countImgNumber.push_back(zero);//零号位，不计入
    for(int i = 1; i <= imgKind; ++i){
        std::vector<Point*> tmp;
        for(int j = 1; j <= eachImgNum; ++j){
            Point* img = new Point(0,0,0,0,0);
            if(i == 1 && j <= colNum){
                logicMap[i][j].kind = 1;
                img->row = 1; img->col = j;
                img->kind = 1;
                img->x = rowToX(1); img->y = colToY(j);
                tmp.push_back(img);
                continue;
            }
            while(true){
                int r = std::rand()%rowNum + 1;
                int c = std::rand()%colNum + 1;
                if(!logicMap[r][c].kind){
                    logicMap[r][c].kind = i;
                    img->row = r, img->col = c;
                    img->kind = i;
                    img->x = rowToX(r), img->y = colToY(c);
                    tmp.push_back(img);
                    break;
                }
            }
        }
        countImgNumber.push_back(tmp);
    }
}

//创建实体地图
void GameArea::paintMap()
{
    for(int i = 0; i <= rowNum+1; ++i){
        std::vector<QPushButton*> tmp;
        for(int j = 0; j <= colNum+1; ++j){
            QPushButton *btn = new QPushButton(this);
            int kind = logicMap[i][j].kind;
            //qDebug()<<logicMap[i][j].x<<QString( )<<logicMap[i][j].y;
            if(!kind){
                btn->setGeometry(logicMap[i][j].x,logicMap[i][j].y,imgSize,imgSize);
                btn->setFlat(true);
                btn->setStyleSheet("background:transparent");
                btn->show();
            }
            else{
                btn->setGeometry(logicMap[i][j].x,logicMap[i][j].y,imgSize,imgSize);
                QString path = QString(":/image/") + QString::number(kind) + QString(".jpg");
                QIcon ico(path);
                btn->setIcon(ico);
                btn->setIconSize(QSize(imgSize,imgSize));
                btn->show();
            }
            tmp.push_back(btn);
        }
        buttonMap.push_back(tmp);
    }
}
void GameArea::paintPlayingMap()
{
    bool finished = true;
    for(int i = 0; i <= rowNum + 1; ++i){
        for(int j = 0; j <= colNum + 1; ++j){
            int kind = logicMap[i][j].kind;
            QPushButton *btn = buttonMap[i][j];
            if(!kind){
                btn->setFlat(true);
                btn->setIconSize(QSize(0,0));
                btn->setStyleSheet("background:transparent");
            }
            else if(kind > 0){
                finished = false;
                QString path = QString(":/image/") + QString::number(kind) + QString(".jpg");
                QIcon ico(path);
                btn->setIcon(ico);
                if(canHint && (sol1 == btn || sol2 == btn)){
                    btn->setStyleSheet("QPushButton{padding: 5px;border: 5px solid orange}");
                    btn->setIconSize(QSize(imgSize-5,imgSize-5));
                    continue;
                }
                if(logicMap[i][j].chosen) paintBlockCrashed(btn,logicMap[i][j].chosen);
                else paintBlockUncrashed(btn);
            }
            else{
                QString path = QString(":/image/") + QString::number(kind) + QString(".jpg");
                QIcon ico(path);
                btn->setIcon(ico);
                btn->setIconSize(QSize(imgSize,imgSize));
                btn->setFlat(false);
            }
            btn->show();
        }
    }
    if(finished){
        emit gameIsOver();
    }
}
//创建人物
void GameArea::initPaintPlayer()
{
    player1->randPlayerPosition(this);
    player1Img->setGeometry(player1->x,player1->y,imgSize,imgSize);
    player1Img->setIcon(QIcon(":/image/9.jpg"));
    player1Img->setIconSize(QSize(imgSize,imgSize));
    player1Img->show();
    accessMap1[player1->row][player1->col] = true;
    accessToPlayer(player1->row,player1->col,1);

    if(hasTwoPlayers){
        while(1){
            player2->randPlayerPosition(this);
            if(!(player2->x != player1->x && player2->y != player1->y)) break;
        }
        qDebug("player2");
        player2Img->setGeometry(player2->x,player2->y,imgSize,imgSize);
        player2Img->setIcon(QIcon(":/image/10.jpg"));
        player2Img->setIconSize(QSize(imgSize,imgSize));
        player2Img->show();
        accessMap2[player2->row][player2->col] = true;
        accessToPlayer(player2->row,player2->col,2);
    }
}
void GameArea::paintPlayer()
{
    if(player1)player1Img->move(player1->x,player1->y);
    if(hasTwoPlayers){
        player2Img->move(player2->x,player2->y);
    }
}
//打印提示地图
void GameArea::paintHint()
{
    canSolve(sol1,sol2);
    qDebug("solved");
    qDebug()<<logicMap[xToRow(sol1->x())][yToCol(sol1->y())].kind << logicMap[xToRow(sol2->x())][yToCol(sol2->y())].kind;
    sol1->setStyleSheet("QPushButton{padding: 5px;border: 5px solid yellow}");
    sol2->setStyleSheet("QPushButton{padding: 5px;border: 5px solid yellow}");
    qDebug("set");
}


//按键事件
void GameArea::keyPressEvent(QKeyEvent *event)
{
    switch (event -> key()) {
    //up down left right
    case Qt::Key_W:
        ifKeyPressed = true;
        keyPressKind = 1;
        break;
    case Qt::Key_S:
        ifKeyPressed = true;
        keyPressKind = 2;
        break;
    case Qt::Key_A:
        ifKeyPressed = true;
        keyPressKind = 3;
        break;
    case Qt::Key_D:
        ifKeyPressed = true;
        keyPressKind = 4;
        break;
    case Qt::Key_Up:
        if(hasTwoPlayers){
            ifKeyPressed = true;
            keyPressKind = -1;
        }
        break;
    case Qt::Key_Down:
        if(hasTwoPlayers){
            ifKeyPressed = true;
            keyPressKind = -2;
        }
        break;
    case Qt::Key_Left:
        if(hasTwoPlayers){
            ifKeyPressed = true;
            keyPressKind = -3;
        }
        break;
    case Qt::Key_Right:
        if(hasTwoPlayers){
            ifKeyPressed = true;
            keyPressKind = -4;
        }
        break;
    }
    if(player1->dizzy){
        if(keyPressKind == 1) keyPressKind = 2;
        else if(keyPressKind == 2) keyPressKind = 1;
        else if(keyPressKind == 3) keyPressKind = 4;
        else if(keyPressKind == 4) keyPressKind = 3;
    }
    else if(hasTwoPlayers && player2->dizzy){
        if(keyPressKind == -1) keyPressKind = -2;
        else if(keyPressKind == -2) keyPressKind = -1;
        else if(keyPressKind == -3) keyPressKind = -4;
        else if(keyPressKind == -4) keyPressKind = -3;
    }
    if(player1->freeze){
        if(keyPressKind>0) keyPressKind = 0;
    }
    else if(hasTwoPlayers && player2->freeze){
        if(keyPressKind<0) keyPressKind = 0;
    }
    judgeKey();
}
//按键分类判断,进而释放撞击信号
void GameArea::judgeKey()
{
    int tmp = 0;
    switch (keyPressKind) { // WSAD
    case 1:
        tmp = (player1->col-1 < 0)? colNum+1 : player1->col-1;
        if(player1->judgeCrashBlock(player1->row,tmp,this)){
            player1->col = tmp;
            player1->y = colToY(tmp);
        }
        break;
    case -1:
        tmp = (player2->col-1 < 0)? colNum+1 : player2->col-1;
        if(player2->judgeCrashBlock(player2->row,tmp,this)){
            player2->col = tmp;
            player2->y = colToY(tmp);
        }
        break;
    case 2:
        tmp = (player1->col+1 > colNum+1)? 0 : player1->col+1;
        if(player1->judgeCrashBlock(player1->row,tmp,this)){
            player1->col = tmp;
            player1->y = colToY(tmp);
        }
        break;
    case -2:
        tmp = (player2->col+1 > colNum+1)? 0 : player2->col+1;
        if(player2->judgeCrashBlock(player2->row,tmp,this)){
            player2->col = tmp;
            player2->y = colToY(tmp);
        }
        break;
    case 3:
        tmp = (player1->row-1 < 0)? rowNum+1 : player1->row-1;
        if(player1->judgeCrashBlock(tmp,player1->col,this)){
            player1->row = tmp;
            player1->x = rowToX(tmp);
        }
        break;
    case -3:
        tmp = (player2->row-1 < 0)? rowNum+1 : player2->row-1;
        if(player2->judgeCrashBlock(tmp,player2->col,this)){
            player2->row = tmp;
            player2->x = rowToX(tmp);
        }
        break;
    case 4:
        tmp = (player1->row+1 > rowNum+1)? 0 : player1->row+1;
        if(player1->judgeCrashBlock(tmp,player1->col,this)){
            player1->row = tmp;
            player1->x = rowToX(tmp);
        }
        break;
    case -4:
        tmp = (player2->row+1 > rowNum+1)? 0 : player2->row+1;
        if(player2->judgeCrashBlock(tmp,player2->col,this)){
            player2->row = tmp;
            player2->x = rowToX(tmp);
        }
        break;
    }
    if(hasTwoPlayers) judgeProperty(player1->row, player1->col, player2->row, player2->col);
    else judgeProperty(player1->row, player1->col);
    paintPlayer();
}

//绘制选中图标、进行逻辑判断、更新信息SLOT
void GameArea::blockCrashed1(int row, int col)
{
    blockCrashed(row,col,1);
}
void GameArea::blockCrashed2(int row, int col)
{
    blockCrashed(row,col,2);
}
void GameArea::blockCrashed(int row, int col,int type)
{
    QPushButton *btn = buttonMap[row][col];//crashed block
    //qDebug()<< row << col << type;
    //按两次一样的取消选中DONE
    if(type == 1 && player1->prevBlock == btn){
        logicMap[row][col].chosen = 0;
        player1->prevBlock = nullptr;
        return;
    }
    else if(type == 2 && player2->prevBlock == btn){
        logicMap[row][col].chosen = 0;
        player2->prevBlock = nullptr;
        return;
    }
    if(type == 1) prevBlock = player1->prevBlock;
    else prevBlock = player2->prevBlock;
    if(prevBlock){
        //qDebug()<<xToRow(prevBlock->x())<<yToCol(prevBlock->y());
        logicMap[xToRow(prevBlock)][yToCol(prevBlock)].chosen = 0;

        if(canConnect(btn,prevBlock)){
            paintLink(xToRow(btn),yToCol(btn),xToRow(prevBlock),yToCol(prevBlock));
            //更新玩家分数
            if(type == 1) player1->score += lcdTime-minusTime;
            else if(type == 2) player2->score += lcdTime-minusTime;
            //更新种类计数信息
            Point* tmp = nullptr,* stmp = nullptr;
            int kind = logicMap[row][col].kind;
            for(int i = 0; i < (int)countImgNumber[kind].size(); ++i){
                tmp = countImgNumber[kind][i];
                stmp = countImgNumber[kind].back();
                if((tmp->row == row && tmp->col == col) ||
                   (tmp->row == xToRow(prevBlock) && tmp->col == yToCol(prevBlock))){
                    tmp->row = stmp->row; tmp->col = stmp->col; tmp->x = stmp->x; tmp->y = stmp->y;
                    countImgNumber[kind].pop_back();
                    --i;
                }
            }
            //更新图片信息
            logicMap[row][col].kind = 0;
            logicMap[xToRow(prevBlock)][yToCol(prevBlock)].kind = 0;
            if(type == 1) player1->prevBlock = nullptr;
            else player2->prevBlock = nullptr;
            //更新是否有解
            if(!canSolve(sol1,sol2)) {
                emit gameIsOver();
            }
            return;
        }
        logicMap[row][col].chosen = type;
        if(type == 1)player1->prevBlock = btn;
        else player2->prevBlock = btn;
    }
    else{
        //qDebug()<<type;
        logicMap[xToRow(btn)][yToCol(btn)].chosen = type;
        if(type == 1) player1->prevBlock = btn;
        else player2->prevBlock = btn;
    }
    //prevBlock = nullptr;
}

//能否相连逻辑判断DONE
bool GameArea::canConnect(QPushButton* cur,QPushButton* prev)
{
    linkPahtPoints.clear();
    int row1 = xToRow(cur), col1 = yToCol(cur);
    int row2 = xToRow(prev), col2 = yToCol(prev);
    if(!(logicMap[row1][col1].kind == logicMap[row2][col2].kind)) return false;
    if(connectOneLine(row1,col1,row2,col2)) {qDebug("oneLine"); return true;}
    if(connectTwoLines(row1,col1,row2,col2)) {qDebug("twoLines"); return true;}
    if(connectThreeLines(row1,col1,row2,col2)) {qDebug("threeLines"); return true;}
    return false;
}
bool GameArea::canConnect(int row1, int col1, int row2, int col2)
{
    linkPahtPoints.clear();
    if(logicMap[row1][col1].kind != logicMap[row2][col2].kind) return false;
    if(connectOneLine(row1,col1,row2,col2) ||
       connectTwoLines(row1,col1,row2,col2) ||
       connectThreeLines(row1,col1,row2,col2)){
        linkPahtPoints.clear();
        return true;
    }
    return false;
}
bool GameArea::connectOneLine(int row1, int col1, int row2, int col2)
{
    if(row1 == row2 && col1 == col2) return false;
    if(row1 == row2){
        for(int i = std::min(col1,col2)+1; i < std::max(col1,col2); ++i){
            if(logicMap[row1][i].kind > 0) return false;
        }
        return true;
    }
    else if(col1 == col2){
        for(int i = std::min(row1,row2)+1; i < std::max(row1,row2); ++i){
            if(logicMap[i][col1].kind > 0) return false;
        }
        return true;
    }
    return false;
}
bool GameArea::connectTwoLines(int row1, int col1, int row2, int col2)
{
    if( logicMap[row1][col2].kind == 0 &&
        connectOneLine(row1,col1,row1,col2) && connectOneLine(row1,col2,row2,col2)){
        linkPahtPoints.push_back(Point(row1,col2,0,rowToX(row1),colToY(col2)));
        return true;
    }
    if( logicMap[row2][col1].kind == 0 &&
        connectOneLine(row1,col1,row2,col1) && connectOneLine(row2,col1,row2,col2)){
        linkPahtPoints.push_back(Point(row2,col1,0,rowToX(row1),colToY(col1)));
        return true;
    }
    return false;
}
bool GameArea::connectThreeLines(int row1, int col1, int row2, int col2)
{
    for(int i = 0; i <= rowNum+1; ++i){
        if( logicMap[i][col1].kind == 0 &&
            connectOneLine(row1,col1,i,col1) && connectTwoLines(i,col1,row2,col2)){
            linkPahtPoints.push_back(Point(i,col1,0,rowToX(i),colToY(col1)));
            return true;
        }
    }
    for(int i = 0; i <= colNum+1; ++i){
        if( logicMap[row1][i].kind == 0 &&
            connectOneLine(row1,col1,row1,i) && connectTwoLines(row1,i,row2,col2)){
            linkPahtPoints.push_back(Point(row1,i,0,rowToX(row1),colToY(i)));
            return true;
        }
    }
    return false;
}


//判断是否还有可以消除的 TODO
bool GameArea::canSolve(QPushButton*& sol1, QPushButton*& sol2)
{
    Q_UNUSED(sol1); Q_UNUSED(sol2);
    for(int i = 0; i <= rowNum+1; ++i){
        for(int j = 0; j <= colNum+1; ++j){
            accessMap1[i][j] = false;
            if(hasTwoPlayers) accessMap2[i][j] = false;
        }
    }
    accessMap1[player1->row][player1->col] = true;
    accessToPlayer(player1->row,player1->col,1);
    if(hasTwoPlayers){
        accessMap2[player2->row][player2->col] = true;
        accessToPlayer(player2->row,player2->col,2);
    }
    int jr = 0, jc = 0, kr = 0, kc = 0;
    for(int i = 1; i <= imgKind; ++i){
        for(int j = 0; j < (int)countImgNumber[i].size(); ++j){
            jr = countImgNumber[i][j]->row, jc = countImgNumber[i][j]->col;

            if(hasTwoPlayers && (!playerCanHandle(jr,jc,2) || !playerCanHandle(jr,jc,1))) continue;
            else if(!hasTwoPlayers && !playerCanHandle(jr,jc,1)) continue;

            for(int k = j+1; k < (int)countImgNumber[i].size(); ++k){
                kr = countImgNumber[i][k]->row, kc = countImgNumber[i][k]->col;
                if(hasTwoPlayers && playerCanHandle(kr,kc,1) && playerCanHandle(kr,kc,2) && canConnect(jr,jc,kr,kc)) {
                    sol1 = buttonMap[jr][jc], sol2 = buttonMap[kr][kc];
                    return true;
                }
                else if(!hasTwoPlayers && playerCanHandle(kr,kc,1) && canConnect(jr,jc,kr,kc)){
                    sol1 = buttonMap[jr][jc], sol2 = buttonMap[kr][kc];
                    return true;
                }
            }
        }
    }
    return false;
}
//设置可到达地图
void GameArea::accessToPlayer(int row,int col,int type)
{
    if(type == 1){
        if(!logicMap[(row+1)%(rowNum+2)][col].kind && !accessMap1[(row+1)%(rowNum+2)][col]){
            accessMap1[(row+1)%(rowNum+2)][col] = true;
            accessToPlayer((row+1)%(rowNum+2),col,type);
        }
        if(!logicMap[(row-1+rowNum+2)%(rowNum+2)][col].kind && !accessMap1[(row-1+rowNum+2)%(rowNum+2)][col]){
            accessMap1[(row-1+rowNum+2)%(rowNum+2)][col] = true;
            accessToPlayer((row-1+rowNum+2)%(rowNum+2),col,type);
        }
        if(!logicMap[row][(col+1)%(colNum+2)].kind && !accessMap1[row][(col+1)%(colNum+2)]){
            accessMap1[row][(col+1)%(colNum+2)] = true;
            accessToPlayer(row,(col+1)%(colNum+2),type);
        }
        if(!logicMap[row][(col-1+colNum+2)%(colNum+2)].kind && !accessMap1[row][(col-1+colNum+2)%(colNum+2)]){
            accessMap1[row][(col-1+colNum+2)%(colNum+2)] = true;
            accessToPlayer(row,(col-1+colNum+2)%(colNum+2),type);
        }
    }
    else{
        if(!logicMap[(row+1)%(rowNum+2)][col].kind && !accessMap2[(row+1)%(rowNum+2)][col]){
            accessMap2[(row+1)%(rowNum+2)][col] = true;
            accessToPlayer((row+1)%(rowNum+2),col,type);
        }
        if(!logicMap[(row-1+rowNum+2)%(rowNum+2)][col].kind && !accessMap2[(row-1+rowNum+2)%(rowNum+2)][col]){
            accessMap2[(row-1+rowNum+2)%(rowNum+2)][col] = true;
            accessToPlayer((row-1+rowNum+2)%(rowNum+2),col,type);
        }
        if(!logicMap[row][(col+1)%(colNum+2)].kind && !accessMap2[row][(col+1)%(colNum+2)]){
            accessMap2[row][(col+1)%(colNum+2)] = true;
            accessToPlayer(row,(col+1)%(colNum+2),type);
        }
        if(!logicMap[row][(col-1+colNum+2)%(colNum+2)].kind && !accessMap2[row][(col-1+colNum+2)%(colNum+2)]){
            accessMap2[row][(col-1+colNum+2)%(colNum+2)] = true;
            accessToPlayer(row,(col-1+colNum+2)%(colNum+2),type);
        }
    }
}
//判断玩家可操作位置
bool GameArea::playerCanHandle(int row, int col, int type)
{
    if(!logicMap[row][col].kind) return false;
    if(type == 1){
        return(accessMap1[row+1][col] || accessMap1[row-1][col] ||
               accessMap1[row][col+1] || accessMap1[row][col-1]);
    }
    else{
        return(accessMap2[row+1][col] || accessMap2[row-1][col] ||
               accessMap2[row][col+1] || accessMap2[row][col-1]);
    }
}


//绘制、消除连线DONE
void GameArea::paintLink(int row1, int col1, int row2, int col2)
{
    timerPathLink = new QTimer(this);
    connect(timerPathLink,SIGNAL(timeout()),this,SLOT(erasePathLink()));
    timerPathLink->start(20);
    if(!linkPahtPoints.size()){
//        qDebug("paintOneLink");
        paintOneLink(row1,col1,row2,col2);
    }
    else if(linkPahtPoints.size() == 1){
//        qDebug("paintTwoLinks");
        Point tmp = linkPahtPoints.back();
        qDebug()<<tmp.row<<tmp.col;
        paintOneLink(row1,col1,tmp.row,tmp.col);
        paintOneLink(tmp.row,tmp.col,row2,col2);
        linkPahtPoints.pop_back();
    }
    else{
//        qDebug("paintThreeLinks");
        Point tmp1 = linkPahtPoints.back();
        linkPahtPoints.pop_back();
        Point tmp2 = linkPahtPoints.back();
        linkPahtPoints.pop_back();
//        qDebug()<<tmp1.row<<tmp1.col;
//        qDebug()<<tmp2.row<<tmp2.col;
        paintOneLink(row1,col1,tmp1.row,tmp1.col);
        paintOneLink(tmp1.row,tmp1.col,tmp2.row,tmp2.col);
        paintOneLink(row2,col2,tmp2.row,tmp2.col);
    }
}
void GameArea::paintOneLink(int row1, int col1, int row2, int col2)
{
    if(row1 == row2){
        for(int i = std::min(col1,col2); i < std::max(col1,col2); ++i){
            QPushButton *pathPoint = new QPushButton(this);
            pathPoint -> setGeometry(rowToX(row1) + (imgSize - pathSize)/2,
                                     colToY(i) + (imgSize - pathSize)/2,
                                     pathSize,imgSize);
            pathPoint->setStyleSheet("background-color: rgb(0,255,0)");
            linkButtons.push_back(pathPoint);
            pathPoint->show();
        }
    }
    else{
        for(int i = std::min(row1,row2); i < std::max(row1,row2); ++i){
            QPushButton *pathPoint = new QPushButton(this);
            pathPoint -> setGeometry(rowToX(i) + (imgSize - pathSize)/2,
                                     colToY(col1) + (imgSize - pathSize)/2,
                                     imgSize,pathSize);
            pathPoint->setStyleSheet("background-color: rgb(0,255,0)");
            linkButtons.push_back(pathPoint);
            pathPoint->show();
        }
    }
}
void GameArea::erasePathLink()
{
    while(linkButtons.size()){
        linkButtons.back()->setFlat(true);
        delete linkButtons.back();
        linkButtons.pop_back();
    }
    delete timerPathLink;
}


//绘制/取消按钮选中图案DONE
void GameArea::paintBlockCrashed(QPushButton *btn,int type)
{
    if(type == 1)
        btn->setStyleSheet("QPushButton{padding: 5px;border: 5px solid green}");
    else if(type == 2)
        btn->setStyleSheet("QPushButton{padding: 5px;border: 5px solid red}");
    btn->setIconSize(QSize(imgSize-5,imgSize-5));
}
void GameArea::paintBlockUncrashed(QPushButton *btn)
{
    btn->setStyleSheet("QPushButton{padding:0px}");
    btn->setIconSize(QSize(imgSize,imgSize));
}


//处理时间
void GameArea::timerEvent(QTimerEvent *event)
{
    if(!(id == event->timerId())) return;
    if(isPaused) return;
    if(lcdTime-minusTime >= 0){
        ui->lcdNumber->display(QString::number(lcdTime-minusTime));
        ++minusTime;
    }
    else{
        emit gameIsOver();
        return;
    }
    if(!((lcdTime-minusTime)%PROTIME)){
//        qDebug()<<propertyKind%3+1;
        if(!hasTwoPlayers)
            addProperty(propertyKind%4+1);
        else
            addProperty(propertyKind%5+1 >= 3? propertyKind%5+2 : propertyKind%5+1);
        ++propertyKind;
    }
}

//处理道具
void GameArea::addProperty(int propertyKind)
{
    int i,j;
    while(1){
        i = std::rand()%(rowNum + 2);
        j = std::rand()%(colNum + 2);
        if(hasTwoPlayers){
            if(!logicMap[i][j].kind && accessMap1[i][j] && accessMap2[i][j] &&
              (i != player1->row && j != player1->col) && (i != player2->row && j != player2->col)){
                logicMap[i][j].kind = -propertyKind;
                break;
            }
        }
        else{
            if(!logicMap[i][j].kind && accessMap1[i][j] &&
              (i != player1->row && j != player1->col)){
                logicMap[i][j].kind = -propertyKind;
                break;
            }
        }
    }

}
void GameArea::judgeProperty(int row, int col)
{
    QPushButton* btn = nullptr;
    switch(logicMap[row][col].kind){
    case -1://+1s
        minusTime -= 30;
        break;
    case -2://shuffle
        btn = player1->prevBlock;
        if(btn)logicMap[xToRow(btn->x())][yToCol(btn->y())].chosen = 0;
        player1->prevBlock = nullptr;
        shuffle();
        break;
    case -3://flash
        timerFlash->start(5000);
        canFlash = true;
        break;
    case -4://hint
        timerHint->start(10000);
        canHint = true;
        break;
    }
    logicMap[row][col].kind = 0;
}
void GameArea::judgeProperty(int row1, int col1, int row2, int col2){
    QPushButton* btn1 = nullptr;
    QPushButton* btn2 = nullptr;
    switch (logicMap[row1][col1].kind) {
    case -1://+1s
        minusTime -= 30;
        break;
    case -2://shuffle
        btn1 = player1->prevBlock; btn2 = player2->prevBlock;
        if(btn1)logicMap[xToRow(btn1->x())][yToCol(btn1->y())].chosen = 0;
        if(btn2)logicMap[xToRow(btn2->x())][yToCol(btn2->y())].chosen = 0;
        player1->prevBlock = nullptr;
        player2->prevBlock = nullptr;
        shuffle();
        break;
    case -4://hint
        qDebug("hint");
        timerHint->start(10000);
        canHint = true;
        break;
    case -5://freeze
        timerFreeze->start(3000);
        player2->freeze = true;
        break;
    case -6://dizzy
        timerDizzy->start(10000);
        player2->dizzy = true;
        break;
    }
    switch (logicMap[row2][col2].kind) {
    case -1://+1s
        minusTime -= 30;
        break;
    case -2://shuffle
        btn1 = player1->prevBlock; btn2 = player2->prevBlock;
        if(btn1)logicMap[xToRow(btn1->x())][yToCol(btn1->y())].chosen = 0;
        if(btn2)logicMap[xToRow(btn2->x())][yToCol(btn2->y())].chosen = 0;
        player1->prevBlock = nullptr;
        player2->prevBlock = nullptr;
        shuffle();
        break;
    case -4://hint
        qDebug("hint");
        timerHint->start(10000);
        canHint = true;
        break;
    case -5://freeze
        timerFreeze->start(3000);
        player1->freeze = true;
        break;
    case -6://dizzy
        timerDizzy->start(10000);
        player1->dizzy = true;
        break;
    }
    logicMap[row1][col1].kind = 0;
    logicMap[row2][col2].kind = 0;
}
//道具flash 处理鼠标点击
void GameArea::buttonClicked(int row, int col)
{
    //qDebug()<<row<<col;
//    if(!canFlash) qDebug("flashed");
    int newRow = player1->row, newCol = player1->col;
    if(canFlash){
        if(logicMap[row][col].kind <= 0 && accessMap1[row][col]){//空白 可以到达
            newRow = row, newCol = col;
            canFlash = false;
        }
        else if(logicMap[row][col].kind && playerCanHandle(row,col,1)){
            if(!logicMap[(row+1)%(rowNum+2)][col].kind){
                newRow = (row+1)%(rowNum+2), newCol = col;
                prevBlock = nullptr;
                blockCrashed(row,col,1);
                canFlash = false;
            }
            else if(!logicMap[row][(col+1)%(colNum+2)].kind){
                newRow = row, newCol = (col+1)%(colNum+2);
                prevBlock = nullptr;
                blockCrashed(row,col,1);
                canFlash = false;
            }
            else if(!logicMap[(row-1+rowNum+2)%(rowNum+2)][col].kind){
                newRow = (row-1+rowNum+2)%(rowNum+2);
                newCol = col;
                prevBlock = nullptr;
                blockCrashed(row,col,1);
                canFlash = false;
            }
            else if(!logicMap[row][(col-1+colNum+2)%(colNum+2)].kind){
                newRow = row;
                newCol = (col-1+colNum+2)%(colNum+2);
                prevBlock = nullptr;
                blockCrashed(row,col,1);
                canFlash = false;
            }
        }
        player1->row = newRow, player1->col = newCol;
        player1->x = rowToX(newRow), player1->y = colToY(newCol);
    }
}

//道具shuffle
void GameArea::shuffle()
{
    int r = 0, c = 0;
    for(int i = 0; i <= rowNum+1; ++i)
        for(int j = 0; j <= colNum+1; ++j)
            if(logicMap[i][j].kind >= 0) logicMap[i][j].kind = 0;

    for(int i = 1; i <= imgKind; ++i){
        for(int j = 0; j < (int)countImgNumber[i].size(); ++j){
            while(1){
                r = std::rand()%rowNum + 1, c = std::rand()%colNum + 1;
                if((!hasTwoPlayers && !logicMap[r][c].kind && (player1->row != r || player1->col != c))
                   || (hasTwoPlayers && !logicMap[r][c].kind && (player1->row != r || player1->col != c)
                   && (player2->row != r || player2->col != c))){
                    logicMap[r][c].kind = i;
                    countImgNumber[i][j]->row = r, countImgNumber[i][j]->col = c;
                    countImgNumber[i][j]->x = rowToX(r), countImgNumber[i][j]->y = colToY(c);
                    break;
                }
            }
        }
    }
    paintLogicMap();
}

//游戏状态
void GameArea::saveGame()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Save");
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QFile::Text)){
        QMessageBox::warning(this,"Warning","Failed to save file: " + file.errorString());
        isPaused = false;
        return;
    }
    QTextStream stream(&file);
    //save the global params
    stream << 'g' << ' '
           << rowNum << ' ' << colNum << ' '
           << imgKind<< ' ' << eachImgNum << ' '
           << imgSize<< ' ' << pathSize << ' '
           << lcdTime << ' ' << minusTime << ' '
           << stX << ' ' << stY << ' ' << hasTwoPlayers << '\n';
    //save logicMap
    stream << 'l' << ' ';
    for(int i = 0; i <= rowNum+1; ++i){
        for(int j = 0; j <= colNum+1; ++j){
            stream << logicMap[i][j].row << ' ' << logicMap[i][j].col << ' '
                   << logicMap[i][j].x << ' ' << logicMap[i][j].y << ' '
                   << logicMap[i][j].kind << ' ' << logicMap[i][j].chosen << ' ';
        }
    }
    stream << '\n';
    //save accessMap
    stream << 'a' << ' ';
    for(int i = 0; i <= rowNum+1; ++i){
        for(int j = 0; j <= colNum+1; ++j){
            stream << accessMap1[i][j] << ' ';
        }
    }
    stream << '\n';
    if(hasTwoPlayers){
        stream << 'b' << ' ';
        for(int i = 0; i <= rowNum+1; ++i){
            for(int j = 0; j <= colNum+1; ++j){
                stream << accessMap2[i][j] << ' ';
            }
        }
        stream << '\n';
    }
    //save the player_info
    if(player1) {
        stream << 'p' << ' '
               << player1->row << ' ' << player1->col << ' '
               << player1->x << ' ' << player1->y << ' '
               << player1->score << ' ' << player1->freeze << ' ' << player1->dizzy << ' ';
        if(player1->prevBlock) stream << xToRow(player1->prevBlock->x()) << ' '
                                      << yToCol(player1->prevBlock->y()) << ' ';
        stream << '\n';
    }
    if(player2) {
        stream << 'q' << ' '
               << player2->row << ' ' << player2->col << ' '
               << player2->x << ' ' << player2->y << ' '
               <<player2->score << ' ' << player2->freeze << ' ' << player2->dizzy << ' ';
        if(player2->prevBlock) stream << xToRow(player2->prevBlock->x()) << ' '
                                      << yToCol(player2->prevBlock->y()) << ' ';
        stream << '\n';
    }
    //save the img_info
    for(int i = 1; i <= imgKind; ++i){
        stream << 'i' << ' ';
        for(int j = 0; j < (int)countImgNumber[i].size(); ++j){
            stream << countImgNumber[i][j]->row << ' ' << countImgNumber[i][j]->col << ' '
                   << countImgNumber[i][j]->x << ' ' << countImgNumber[i][j]->y << ' ';
        }
        stream << '\n';
    }
    //save the superviser
    stream << 's' << ' '
           << canFlash << ' ' << canHint << ' ' << ifKeyPressed << ' ' << 0 << ' ' << keyPressKind << ' ' // 0 means isPaused
           << lcdTime << ' ' << minusTime << ' ' << propertyKind << '\n';
    //ending
    file.close();
    isPaused = false;
}

void GameArea::loadGame()
{
    qDebug("load");
    QString fileName = QFileDialog::getOpenFileName(this,"Open");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)){
        QMessageBox::warning(this,"Warning","Failed to open file: " + file.errorString());
        isPaused = false;
        return;
    }
    clearInfo();
    disConnections();
    QTextStream stream(&file);
    QString text;
    while(!stream.atEnd()){
        text = stream.readLine();
        judgeLoad(text);
    }
    file.close();
    paintMap();
    InitConnections();
    isPaused = false;
    if(hasTwoPlayers){
        ui->scoreTwo->show();
        ui->label_2->show();
        player2Img->setGeometry(player2->x,player2->y,imgSize,imgSize);
        player2Img->setIcon(QIcon(":/image/10.jpg"));
        player2Img->setIconSize(QSize(imgSize,imgSize));
        player2Img->show();
        accessMap2[player2->row][player2->col] = true;
        accessToPlayer(player2->row,player2->col,2);
    }
    else{
        player2Img->setFlat(true);player2Img->setIconSize(QSize(0,0));
        player2Img->setStyleSheet("background:transparent");
        ui->scoreTwo->hide();
        ui->label_2->hide();
    }
}
void GameArea::clearInfo()
{
//    logicMap.clear();
    for(int i = 0; i <= rowNum+1; ++i){
        for(int j = 0; j <= colNum+1; ++j){
            buttonMap[i][j]->hide();
        }
    }
    buttonMap.clear();
//    accessMap1.clear();
//    accessMap2.clear();
    countImgNumber.clear();
    std::vector<Point*> zero;
    countImgNumber.push_back(zero);
    linkPahtPoints.clear();
    linkButtons.clear();
    prevBlock = nullptr;
    player1 = nullptr;
    player2 = nullptr;
//    player1->prevBlock = nullptr;
//    player2->prevBlock = nullptr;
//    hasTwoPlayers = false;
}
void GameArea::judgeLoad(QString text)
{
    std::string str = text.toStdString();
    std::stringstream sstream(str);
    char c;
    int num,num2;
    sstream >> c;
    switch(c){
    case 'g':
        sstream >> rowNum >> colNum
                >> imgKind >> eachImgNum
                >> imgSize >> pathSize
                >> lcdTime >> minusTime
                >> stX >> stY >> hasTwoPlayers;
        //qDebug("g");
        break;
    case 'l':
        for(int i = 0; i <= rowNum+1; ++i){
            for(int j = 0; j <= colNum+1; ++j){
                sstream >> logicMap[i][j].row >> logicMap[i][j].col
                        >> logicMap[i][j].x >> logicMap[i][j].y
                        >> logicMap[i][j].kind >> logicMap[i][j].chosen;
            }
        }
        paintLogicMap();
        //qDebug("l");
        break;
    case 'a':
        for(int i = 0; i <= rowNum+1; ++i){
            for(int j = 0; j <= colNum+1; ++j){
                sstream >> num;
                accessMap1[i][j] = num;
            }
        }
        //wwaqDebug("a");
        break;
    case 'b':
        for(int i = 0; i <= rowNum+1; ++i){
            for(int j = 0; j <= colNum+1; ++j){
                sstream >> num;
                accessMap2[i][j] = num;
            }
        }
        //qDebug("b");
        break;
    case 'p':
        player1 = new Player;
        sstream >> player1->row >> player1->col
                >> player1->x >> player1->y
                >> player1->score;
        if(sstream >> num >> num2){
            player1->prevBlock = buttonMap[num][num2];
        }
        //qDebug("p");
        break;
    case 'q':
        player2 = new Player;
        sstream >> player2->row >> player2->col
                >> player2->x >> player2->y
                >> player2->score;
        if(sstream >> num >> num2){
            player2->prevBlock = buttonMap[num][num2];
        }
        //qDebug("q");
        break;
    case 'i':{
        std::vector<Point*> tmp;
        int r,c,x,y;
        while(sstream >> r >> c >> x >> y){
            Point* p = new Point(r,c,x,y,0);
            tmp.push_back(p);
        }
        countImgNumber.push_back(tmp);
        //qDebug("i");
        break;
    }
    case 's':
        sstream >> num; canFlash = num;
        sstream >> num; canHint = num;
        sstream >> num; ifKeyPressed = num;
        sstream >> num; isPaused = num;
        sstream >> keyPressKind >> lcdTime >> minusTime >> propertyKind;
        //qDebug("s");
        break;
    }
}
//状态传递
int GameArea::get_row() const {return rowNum;}
int GameArea::get_col() const {return colNum;}
int GameArea::get_each_img_num() const {return eachImgNum;}
int GameArea::get_img_kind() const {return imgKind;}
int GameArea::get_img_size() const {return imgSize;}
int GameArea::get_path_size() const {return pathSize;}
int GameArea::get_stx() const {return stX;}
int GameArea::get_sty() const {return stY;}

//进行坐标<->行列转换DONE
int GameArea::xToRow(QPushButton *btn)
{
    return (btn->geometry().x() - stX) / imgSize;
}
int GameArea::yToCol(QPushButton *btn)
{
    return (btn->geometry().y() - stY) / imgSize;
}
int GameArea::xToRow(int x)
{
    if((x-stX)/imgSize >= 0 && (x-stX)/imgSize <= rowNum+1)
        return (int)(x-stX)/imgSize;
    else return -1;
}
int GameArea::yToCol(int y)
{
    if((y-stY)/imgSize >= 0 && (y-stY)/imgSize <= colNum +1)
        return (y-stY)/imgSize;
    else return -1;
}
int GameArea::rowToX(int row)
{
    return row*imgSize + stX;
}
int GameArea::colToY(int col)
{
    return col*imgSize + stY;
}

//显示关闭窗口
void GameArea::showCloseWindow()
{
    if(!hasTwoPlayers){
        if(lcdTime - minusTime) closeWindow->judgeState(3);
        else closeWindow->judgeState(4);
    }
    else{
        if(player1->score > player2->score) closeWindow->judgeState(1);
        else if(player1->score == player2->score) closeWindow->judgeState(5);
        else closeWindow->judgeState(2);
    }
    closeWindow->show();
}

void GameArea::cannotFlash()
{
    //qDebug("cannotflash");
    canFlash = false;
    delete timerFlash;
}

//显示暂停窗口
void GameArea::on_pauseButton_clicked()
{
    isPaused = true;
    pauseWindow->exec();
}


GameArea::~GameArea()
{
    delete ui;
    delete player1;
    delete player2;
    delete player1Img;
    delete player2Img;
    delete timer1;
    delete timerPathLink;
    delete timerFlash;
    delete timerHint;
    delete timerFreeze;
    delete timerDizzy;
    delete prevBlock;
    delete closeWindow;
    delete pauseWindow;
    delete startWindow;
    linkButtons.clear();
    linkPahtPoints.clear();
    logicMap.clear();
    buttonMap.clear();
    countImgNumber.clear();
    accessMap1.clear();
    accessMap2.clear();
}

//测试函数
void GameArea::paintLogicMap(){
    for(int i = 1; i <= rowNum; ++i){
            qDebug() << logicMap[i][1].kind<<logicMap[i][2].kind<<logicMap[i][3].kind<<
                        logicMap[i][4].kind<<logicMap[i][5].kind<<logicMap[i][6].kind;

    }
}
