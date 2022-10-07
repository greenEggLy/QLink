#include "simpletest.h"

SimpleTest::SimpleTest(QObject *parent) : QObject(parent)
{

}

void SimpleTest::testLink()
{
    GameArea game;
    QVERIFY(game.canConnect(1,1,1,2) == true);
    QVERIFY(game.canConnect(1,1,1,1) == false);
    QVERIFY(game.canConnect(1,1,1,3) == true);
}

//QTEST_MAIN(SimpleTest)
