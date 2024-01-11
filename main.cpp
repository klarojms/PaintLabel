#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    ERRCODE err = BCLicenseMeW(
        L"Mem: V.L. LIMITRONIC, S.L. ES-12006",
        eLicKindDeveloper,
        1,
        L"2354B6AC997E6C2644E75FC8034E96F2",
        eLicProd2D);

    w.show();
    return a.exec();
}
