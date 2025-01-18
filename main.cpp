#include "speedpaws.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/img/imgs/icon.png"));
    SpeedPaws w;
    w.showMaximized();
    return a.exec();
}
