
#include "MainWindow.h"
#ifdef LINUX
#include <QApplication>
#else
#include <QtWidgets/QApplication>
#endif


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
