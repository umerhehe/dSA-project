#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set application styling context if needed globally,
    // though we already handled specific styles in mainwindow.cpp
    a.setStyle("Fusion");

    MainWindow w;
    w.setWindowTitle("TSP Route Optimization Dashboard");
    w.show();

    return a.exec();
}