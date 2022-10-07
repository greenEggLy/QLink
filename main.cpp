#include "gamearea.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GameArea w;
    w.show();
    return a.exec();
}
