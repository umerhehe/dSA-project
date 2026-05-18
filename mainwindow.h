#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QPointF>
#include <QList>
#include <QGraphicsRectItem>
#include <QLineEdit>
#include "tsp_graph.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnLoad_clicked();
    void on_btnRun_clicked();
    void on_btnMembers_clicked();
    void on_btnRandom_clicked();
    void on_cmbRouteSelect_currentIndexChanged(int index);
    void updateCarAnimation();
    void updateChartAnimation();
    void processCommand(); // New slot for the command bar

private:
    Ui::MainWindow *ui;
    TSPGraph *graph;
    QGraphicsScene *scene;
    QGraphicsScene *chartScene;

    int *rBM, *rBL, *rGM, *rGL, *rOM, *rOL;
    int currentNumCities;
    int startNode; // Tracks the chosen starting point

    // UI Elements
    QLineEdit *txtCommand;

    // Route Animation Variables
    QTimer *carTimer;
    QGraphicsTextItem *carItem;
    QList<QPointF> currentPath;
    QList<int> currentWeights;
    int animIndex;
    qreal animProgress;

    // Progressive Road Drawing Variables
    QGraphicsLineItem *animRoadShadow;
    QGraphicsLineItem *animRoad;
    QGraphicsLineItem *animRoadDash;
    QColor currentRouteColor;

    // Chart Animation Variables
    QTimer *chartTimer;
    struct ChartBar {
        QGraphicsRectItem* rect;
        QGraphicsTextItem* text;
        double targetWidth;
        double currentWidth;
        double targetVal;
        QColor color;
    };
    QList<ChartBar> activeBars;

    void drawGraph(int* route, int numCities, QColor routeColor);
    void setupTable();
    void drawCharts(qint64 tBM, qint64 tBL, qint64 tGM, qint64 tGL, qint64 tOM, qint64 tOL, int costB, int costG, int costO);
    QString formatRoute(int* route, int n);
};
#endif // MAINWINDOW_H