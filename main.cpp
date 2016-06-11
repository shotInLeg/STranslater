#include "stranslater.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    STranslater w;
    w.show();

    return a.exec();
}
