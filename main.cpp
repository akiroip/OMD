#include "omd.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OMD w;
    w.show();
    return a.exec();
}
