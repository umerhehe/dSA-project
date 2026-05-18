#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <qmath.h>
#include <QRandomGenerator>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), graph(nullptr),
    rBM(nullptr), rBL(nullptr), rGM(nullptr), rGL(nullptr), rOM(nullptr), rOL(nullptr), currentNumCities(0), startNode(0)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this); chartScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene); ui->chartView->setScene(chartScene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setInteractive(true);

    setupTable();
    ui->splitter->setSizes({500, 600});

    this->setStyleSheet("QMainWindow { background-color: #0d1117; }"
                        "QLabel { color: #c9d1d9; font-weight: bold; font-size: 14px; }"
                        "QLabel#titleLabel { font-size: 36px; color: #58a6ff; }"
                        "QPushButton { background-color: #238636; color: white; border-radius: 6px; padding: 10px 16px; font-weight: bold; transition: all 0.2s; }"
                        "QPushButton:hover { background-color: #2ea043; border: 1px solid #3fb950; }"
                        "QPushButton:pressed { background-color: #1a5c25; padding-top: 12px; padding-bottom: 8px; }"
                        "QComboBox { background-color: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 6px; color: #58a6ff; font-weight: bold; }"
                        "QTextEdit, QTableWidget { background-color: #161b22; color: #c9d1d9; border: 1px solid #30363d; border-radius: 8px; gridline-color: #30363d; }"
                        "QHeaderView::section { background-color: #21262d; color: #58a6ff; font-weight: bold; border: 1px solid #30363d; padding: 4px; }");

    QPixmap logo("logo.jpg");
    if (!logo.isNull()) ui->logoLabel->setPixmap(logo.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->cmbRouteSelect->addItems({"Select Route View...", "🔴 Brute Force (Matrix)", "🔴 Brute Force (List)", "🔵 NN (Matrix)", "🔵 NN (List)", "🟢 2-Opt (Matrix)", "🟢 2-Opt (List)"});

    carTimer = new QTimer(this);
    connect(carTimer, &QTimer::timeout, this, &MainWindow::updateCarAnimation);

    chartTimer = new QTimer(this);
    connect(chartTimer, &QTimer::timeout, this, &MainWindow::updateChartAnimation);

    QPushButton *btnRandom = new QPushButton("🎲 Gen Random Graph", this);
    ui->horizontalLayout->insertWidget(2, btnRandom);
    connect(btnRandom, &QPushButton::clicked, this, &MainWindow::on_btnRandom_clicked);

    // Dynamic Command Bar Insertion
    txtCommand = new QLineEdit(this);
    txtCommand->setPlaceholderText("Type 'start from A' & press Enter");
    txtCommand->setStyleSheet("QLineEdit { background-color: #0d1117; color: #00ff00; border: 2px solid #238636; border-radius: 6px; padding: 8px; font-weight: bold; font-family: 'Courier New'; }");
    ui->horizontalLayout->insertWidget(4, txtCommand);
    connect(txtCommand, &QLineEdit::returnPressed, this, &MainWindow::processCommand);
}

MainWindow::~MainWindow() {
    if (graph) delete graph;
    if (rBM) delete[] rBM; if (rBL) delete[] rBL;
    if (rGM) delete[] rGM; if (rGL) delete[] rGL;
    if (rOM) delete[] rOM; if (rOL) delete[] rOL;
    delete ui;
}

void MainWindow::setupTable() {
    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->setHorizontalHeaderLabels({"Algorithm", "Type", "Accuracy", "Time (ns)", "Space (B)", "Complexity"});
    ui->tableWidget->setRowCount(6);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

QString MainWindow::formatRoute(int* route, int n) {
    QString str = "";
    for(int i = 0; i <= n; i++) str += QString(QChar('A' + route[i])) + (i < n ? " → " : "");
    return str;
}

void MainWindow::processCommand() {
    if (!graph) return;

    QString cmd = txtCommand->text().trimmed().toUpper();

    if (cmd.startsWith("START FROM ")) {
        QString nodeStr = cmd.mid(11).trimmed();
        if (nodeStr.length() == 1) {
            int idx = nodeStr[0].toLatin1() - 'A';

            if (idx >= 0 && idx < currentNumCities) {
                startNode = idx;
                ui->txtOutput->append("\n✅ COMMAND ACCEPTED: Route calculation starting from node " + nodeStr);
                on_btnRun_clicked();
                return;
            }
        }
    }

    QMessageBox::critical(this, "Error", "That node is not available.");
}

void MainWindow::on_btnRandom_clicked() {
    currentNumCities = QRandomGenerator::global()->bounded(5, 10);
    if (graph) delete graph;
    graph = new TSPGraph(currentNumCities);
    scene->clear(); chartScene->clear(); ui->cmbRouteSelect->setEnabled(false);
    startNode = 0;

    ui->txtOutput->setText("📊 RANDOM GRAPH GENERATED\nCities (V): " + QString::number(currentNumCities) + "\n\nAdjacency Matrix:\n");

    for (int i = 0; i < currentNumCities; i++) {
        QString rowStr = "";
        for (int j = 0; j < currentNumCities; j++) {
            if (i == j) { graph->addEdge(i, j, 0); rowStr += "0\t"; }
            else {
                int weight = QRandomGenerator::global()->bounded(10, 100);
                graph->addEdge(i, j, weight); rowStr += QString::number(weight) + "\t";
            }
        }
        ui->txtOutput->append(rowStr);
    }
    drawGraph(nullptr, currentNumCities, Qt::lightGray);
}

void MainWindow::on_btnLoad_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Matrix", "", "Text Files (*.txt)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&file);
    currentNumCities = in.readLine().toInt();

    if (graph) delete graph;
    graph = new TSPGraph(currentNumCities);
    scene->clear(); chartScene->clear(); ui->cmbRouteSelect->setEnabled(false);
    startNode = 0;

    ui->txtOutput->setText("📊 GRAPH PROPERTIES\nCities (V): " + QString::number(currentNumCities) + "\n\n");
    for (int i = 0; i < currentNumCities; i++) {
        QStringList parts = in.readLine().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (int j = 0; j < currentNumCities && j < parts.size(); j++) {
            graph->addEdge(i, j, parts[j].toInt());
        }
    }
    file.close(); drawGraph(nullptr, currentNumCities, Qt::lightGray);
}

void MainWindow::on_btnRun_clicked() {
    if (!graph) return;
    int n = currentNumCities;
    if (rBM) delete[] rBM; if (rBL) delete[] rBL; if (rGM) delete[] rGM;
    if (rGL) delete[] rGL; if (rOM) delete[] rOM; if (rOL) delete[] rOL;

    rBM = new int[n + 1]; rBL = new int[n + 1]; rGM = new int[n + 1];
    rGL = new int[n + 1]; rOM = new int[n + 1]; rOL = new int[n + 1];

    QElapsedTimer timer;
    int costBM = 0, costBL = 0;
    qint64 tBM = 0, tBL = 0;

    if (n <= 11) {
        timer.start(); costBM = graph->bruteForceTSPMatrix(startNode, rBM); tBM = timer.nsecsElapsed();
        timer.start(); costBL = graph->bruteForceTSPList(startNode, rBL); tBL = timer.nsecsElapsed();
    } else {
        ui->txtOutput->append("\n⚠️ BRUTE FORCE SKIPPED: N > 11 would freeze the application.");
        costBM = -1; costBL = -1;
    }

    timer.start(); int costGM = graph->greedyTSPMatrix(startNode, rGM); qint64 tGM = timer.nsecsElapsed();
    timer.start(); int costGL = graph->greedyTSPList(startNode, rGL); qint64 tGL = timer.nsecsElapsed();
    timer.start(); int costOM = graph->optimizedGreedyTSPMatrix(startNode, rOM); qint64 tOM = timer.nsecsElapsed();
    timer.start(); int costOL = graph->optimizedGreedyTSPList(startNode, rOL); qint64 tOL = timer.nsecsElapsed();

    if (costGM == -1) return;

    int matrixSpace = n * n * sizeof(int);
    int listSpace = (n * sizeof(Node*)) + (graph->edgeCount * sizeof(Node));
    int bestCost = (costBM != -1) ? qMin(costBM, qMin(costGM, costOM)) : qMin(costGM, costOM);

    auto setRow = [&](int row, QString alg, QString type, int cost, qint64 t, int space, QString comp) {
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(alg));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(type));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(cost == -1 ? "Skipped" : (cost <= bestCost ? "🏆 100%" : "Sub-optimal")));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(cost == -1 ? "N/A" : QString::number(t)));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(space)));
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(comp));
    };

    setRow(0, "Brute (Matrix)", "Exact", costBM, tBM, matrixSpace, "O(V!)");
    setRow(1, "Brute (List)", "Exact", costBL, tBL, listSpace, "O(V!)");
    setRow(2, "NN (Matrix)", "Heur.", costGM, tGM, matrixSpace, "O(V^2)");
    setRow(3, "NN (List)", "Heur.", costGL, tGL, listSpace, "O(V^2)");
    setRow(4, "2-Opt (Matrix)", "Imp. Heur.", costOM, tOM, matrixSpace, "O(V^3)");
    setRow(5, "2-Opt (List)", "Imp. Heur.", costOL, tOL, listSpace, "O(V^3)");

    if (costBM != -1) {
        ui->txtOutput->append("\n🔴 BRUTE FORCE ROUTE:\nPath: " + formatRoute(rBM, n) + "\nTravel Cost: " + QString::number(costBM));
    }
    ui->txtOutput->append("\n🔵 NEAREST NEIGHBOR ROUTE:\nPath: " + formatRoute(rGM, n) + "\nTravel Cost: " + QString::number(costGM));
    ui->txtOutput->append("\n🟢 2-OPT ROUTE:\nPath: " + formatRoute(rOM, n) + "\nTravel Cost: " + QString::number(costOM));

    ui->cmbRouteSelect->setEnabled(true);
    if (ui->cmbRouteSelect->currentIndex() == 5) {
        on_cmbRouteSelect_currentIndexChanged(5);
    } else {
        ui->cmbRouteSelect->setCurrentIndex(5);
    }

    drawCharts(tBM, tBL, tGM, tGL, tOM, tOL, costBM, costGM, costOM);
}

void MainWindow::on_cmbRouteSelect_currentIndexChanged(int index) {
    if (!graph || index == 0) return;
    carTimer->stop();

    if ((index == 1 || index == 2) && currentNumCities > 11) {
        QMessageBox::warning(this, "Skipped", "Brute Force was skipped because N > 11.");
        return;
    }

    if (index == 1 || index == 2) drawGraph(index == 1 ? rBM : rBL, currentNumCities, QColor("#ff4444"));
    else if (index == 3 || index == 4) drawGraph(index == 3 ? rGM : rGL, currentNumCities, QColor("#00ffff"));
    else if (index == 5 || index == 6) drawGraph(index == 5 ? rOM : rOL, currentNumCities, QColor("#00ff00"));
}

void MainWindow::drawCharts(qint64 tBM, qint64 tBL, qint64 tGM, qint64 tGL, qint64 tOM, qint64 tOL, int costB, int costG, int costO) {
    chartScene->clear(); activeBars.clear();
    int validCostB = (costB == -1) ? 0 : costB;
    int maxCost = qMax(validCostB, qMax(costG, costO));
    qint64 maxTime = qMax((qint64)1, qMax(tBM, qMax(tBL, qMax(tGM, qMax(tGL, qMax(tOM, tOL))))));
    int barW = 280;

    auto addAnimatedBar = [&](int y, QString label, double val, double maxVal, QColor c) {
        if (val <= 0) return;
        double tWidth = (val / maxVal) * barW; if (tWidth < 5) tWidth = 5;

        QGraphicsRectItem* rect = chartScene->addRect(100, y, 0, 15, QPen(Qt::NoPen), QBrush(c));
        QGraphicsTextItem* tLab = chartScene->addText(label); tLab->setPos(0, y-3); tLab->setDefaultTextColor(Qt::white);
        QGraphicsTextItem* tVal = chartScene->addText("0"); tVal->setPos(105, y-3); tVal->setDefaultTextColor(c);

        activeBars.append({rect, tVal, tWidth, 0.0, val, c});
    };

    QGraphicsTextItem* tTitle = chartScene->addText("⚡ Execution Time (ns)"); tTitle->setDefaultTextColor(QColor("#58a6ff")); tTitle->setFont(QFont("Arial", 11, QFont::Bold));
    addAnimatedBar(25, "Brute(M):", tBM, maxTime, QColor("#ff4444")); addAnimatedBar(45, "Brute(L):", tBL, maxTime, QColor("#ff7777"));
    addAnimatedBar(70, "NN(M):", tGM, maxTime, QColor("#00ffff")); addAnimatedBar(90, "NN(L):", tGL, maxTime, QColor("#77ffff"));
    addAnimatedBar(115, "2-Opt(M):", tOM, maxTime, QColor("#00ff00")); addAnimatedBar(135, "2-Opt(L):", tOL, maxTime, QColor("#77ff77"));

    QGraphicsTextItem* cTitle = chartScene->addText("⛽ Travel Cost"); cTitle->setPos(0, 160); cTitle->setDefaultTextColor(QColor("#58a6ff")); cTitle->setFont(QFont("Arial", 11, QFont::Bold));
    addAnimatedBar(190, "Brute:", validCostB, maxCost, QColor("#ff4444"));
    addAnimatedBar(215, "NN:", costG, maxCost, QColor("#00ffff"));
    addAnimatedBar(240, "2-Opt:", costO, maxCost, QColor("#00ff00"));

    chartTimer->start(16);
}

void MainWindow::updateChartAnimation() {
    bool allDone = true;
    for (int i = 0; i < activeBars.size(); ++i) {
        if (activeBars[i].currentWidth < activeBars[i].targetWidth) {
            allDone = false;
            activeBars[i].currentWidth += (activeBars[i].targetWidth / 30.0);
            if (activeBars[i].currentWidth >= activeBars[i].targetWidth) {
                activeBars[i].currentWidth = activeBars[i].targetWidth;
                activeBars[i].text->setPlainText(QString::number(activeBars[i].targetVal, 'f', 0));
            } else {
                double currentVal = (activeBars[i].currentWidth / activeBars[i].targetWidth) * activeBars[i].targetVal;
                activeBars[i].text->setPlainText(QString::number(currentVal, 'f', 0));
            }
            activeBars[i].rect->setRect(100, activeBars[i].rect->rect().y(), activeBars[i].currentWidth, 15);
            activeBars[i].text->setPos(105 + activeBars[i].currentWidth, activeBars[i].rect->rect().y() - 3);
        }
    }
    if (allDone) chartTimer->stop();
}

void MainWindow::drawGraph(int* route, int numCities, QColor routeColor) {
    scene->clear();
    currentPath.clear();
    currentWeights.clear();
    if (numCities <= 1) return;

    int radius = 220, centerX = 260, centerY = 260;
    struct Point { int x, y; }; Point* points = new Point[numCities];

    for (int i = 0; i < numCities; i++) {
        points[i].x = centerX + radius * qCos(2 * M_PI * i / numCities);
        points[i].y = centerY + radius * qSin(2 * M_PI * i / numCities);
    }

    currentRouteColor = routeColor;

    if (route != nullptr) {
        for (int i = 0; i < numCities; i++) {
            int u = route[i], v = route[i+1];
            currentPath.append(QPointF(points[u].x + 25, points[u].y + 25));
            currentWeights.append(graph->matrix[u][v]);
        }
        currentPath.append(QPointF(points[route[numCities]].x + 25, points[route[numCities]].y + 25));

        // BIGGER GARAGE LOGO (Font 40)
        QGraphicsTextItem* pinItem = scene->addText("🏭");
        pinItem->setFont(QFont("Arial", 40));
        pinItem->setPos(points[route[0]].x - 12, points[route[0]].y - 45); // Shifted to remain centered above node
        pinItem->setZValue(5);
    }

    for (int i = 0; i < numCities; i++) {
        QGraphicsEllipseItem* el = scene->addEllipse(points[i].x, points[i].y, 50, 50, QPen(routeColor.isValid() ? routeColor : QColor("#58a6ff"), 3), QBrush(QColor("#0d1117")));
        el->setZValue(6);
        QGraphicsTextItem* t = scene->addText(QString(QChar('A' + i)));
        t->setPos(points[i].x+14, points[i].y+10);
        t->setDefaultTextColor(Qt::white);
        t->setFont(QFont("Courier New", 14, QFont::Bold));
        t->setZValue(7);
    }
    delete[] points;

    if (route != nullptr) {
        animIndex = 0; animProgress = 0.0;
        animRoadShadow = nullptr; animRoad = nullptr; animRoadDash = nullptr;

        carItem = scene->addText("🚘");
        carItem->setFont(QFont("Arial", 36));
        carItem->setPos(currentPath[0].x() - 25, currentPath[0].y() - 25);
        carItem->setZValue(10);
        carTimer->start(15);
    }
}

void MainWindow::updateCarAnimation() {
    if (currentPath.isEmpty() || animIndex >= currentPath.size() - 1) {
        carTimer->stop(); return;
    }

    QPointF p1 = currentPath[animIndex];
    QPointF p2 = currentPath[animIndex + 1];
    qreal dx = p2.x() - p1.x(); qreal dy = p2.y() - p1.y();
    qreal dist = qSqrt(dx*dx + dy*dy);

    if (animProgress == 0.0) {
        animRoadShadow = scene->addLine(p1.x(), p1.y(), p1.x(), p1.y(), QPen(currentRouteColor, 5, Qt::SolidLine, Qt::RoundCap));
        animRoadShadow->setOpacity(0.3);
        animRoad = scene->addLine(p1.x(), p1.y(), p1.x(), p1.y(), QPen(QColor(30, 30, 30), 3, Qt::SolidLine, Qt::RoundCap));
        animRoadDash = scene->addLine(p1.x(), p1.y(), p1.x(), p1.y(), QPen(QColor("#facc15"), 1, Qt::DashLine));

        animRoadShadow->setZValue(1);
        animRoad->setZValue(2);
        animRoadDash->setZValue(3);
    }

    qreal step = 6.0 / dist;
    animProgress += step;

    qreal clampedProgress = qMin(animProgress, 1.0);
    qreal cx = p1.x() + dx * clampedProgress;
    qreal cy = p1.y() + dy * clampedProgress;

    if (animRoadShadow) animRoadShadow->setLine(p1.x(), p1.y(), cx, cy);
    if (animRoad) animRoad->setLine(p1.x(), p1.y(), cx, cy);
    if (animRoadDash) animRoadDash->setLine(p1.x(), p1.y(), cx, cy);

    carItem->setPos(cx - 25, cy - 25);

    if (animProgress >= 1.0) {
        QGraphicsTextItem* wText = scene->addText(QString::number(currentWeights[animIndex]));
        wText->setPos((p1.x() + p2.x())/2 - 10, (p1.y() + p2.y())/2 - 15);

        // WHITE EDGE NUMBERS
        wText->setDefaultTextColor(Qt::white);
        wText->setFont(QFont("Courier New", 14, QFont::Bold));
        wText->setZValue(8);

        animProgress = 0.0; animIndex++;
        if (animIndex >= currentPath.size() - 1) {
            carTimer->stop(); return;
        }
    }
}

void MainWindow::on_btnMembers_clicked() {
    QMessageBox msgBox;
    msgBox.setStyleSheet("QMessageBox { background-color: #0d1117; color: white; }");
    msgBox.setWindowTitle("About Project");
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText("<h3 style='color: #58a6ff;'>Data Structures & Algorithms</h3><b>Project:</b> TSP Route Optimization Dashboard<br><br><b>Team Members:</b><br>1) MUHAMMAD UMER BIN YASIN - 069<br>2) HUSNAIN ARSHAD - 057<br><br><b>Submitted to:</b><br>Mam. Saima Javad<br><br><b>Description:</b><br>Comparing O(V!) Exact Solutions with O(V^2) Heuristics and O(V^3) 2-Opt optimizations. (Includes full Static vs Dynamic representation comparisons).");
    msgBox.exec();
}