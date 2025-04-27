
#include <sys/stat.h>
#include "mainwindow.h"
#include <QApplication>




int main (int argc, char * argv[]) {

    //PARSE INPUT ARGUMENTS
    QApplication a(argc, argv);
    MainWindow w;
    //w.print_in_textBrowser("hello");
    w.show();


    return a.exec();
}
#include "main.moc"
