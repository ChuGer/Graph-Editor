#include <QtGui>
#include <QLabel>
#include <QList>
#include <QDebug>
#include <QFileDialog>
#include "mainwindow.h"
#include "graphscene.h"
#include "subwindow.h"
#include "edge.h"
#include "graph.h"
#include <math.h>
#include <dialog.h>
#include "viewer.h"
////////////////////////////////////////////////////////////////////////////////////////
// ����������� �������� ����
MainWindow::MainWindow()
{
    // �������� ������� ���������� �������
    mdiArea = new QMdiArea;
    setCentralWidget(mdiArea);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(setActiveSubWindow(QMdiSubWindow*)));
    
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)),
            this, SLOT(setActiveSub(QWidget*)));
    createActions();
    createMenus();
    createToolbars();
    
    newFile();
    activeGraphicsView()->setWindowState(Qt::WindowMaximized);
    
    setWindowTitle("��������� ������ ������������� ������");
    setWindowIcon(QIcon(":/images/editor.png"));
    setWindowState(Qt::WindowMaximized);
    updateRecentFileActions();
}
void MainWindow::setActiveSub(QWidget* w)
{
    qDebug() << "setActiveSub!!!!!!!!!";

    QMdiSubWindow *msw = qobject_cast<QMdiSubWindow*>(w);
    this->setActiveSubWindow(msw);
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ��������������� ������� �������� �������
void MainWindow::subClosed()
{
    this->statusBar()->showMessage("������� ���� �������");

    qDebug() << "subClosed!!!!!!!!!" << mdiArea->subWindowList().count();
    // �������� ���������� ��� �������� ���������� ����
    if (mdiArea->subWindowList().count() == 1)
    {
        this->clearScene();
        this->updateEditToolBar();
        this->statusBar()->showMessage("�����");
    }
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� ����������� �����
SubWindow* MainWindow::createGraphicsView()
{
    ////qDebug() << "createGraphicsView!!!!!!!!!";
    
    // �������� �����
    GraphScene *childScene = new GraphScene(itemMenu,arrowMenu);
    childScene = new GraphScene(itemMenu,arrowMenu);
    childScene->setSceneRect(QRectF(0, 0, 2000, 2000));
    connect(childScene, SIGNAL(addVertexImMatrix(Vertex *)),this, SLOT(addVertexImMatrix(Vertex *)));
    connect(childScene, SIGNAL(addEdgeInMatrix(Edge *)),this, SLOT(addEdgeInMatrix(Edge *)));
    connect(childScene, SIGNAL(setMoveMode()),this, SLOT(setMoveMode()));

    // �������� ����������
    SubWindow *childview = new SubWindow(childScene);
    connect(childview,SIGNAL(subClosed()),this,SLOT(subClosed()));
    connect(childview,SIGNAL(setCurrentFile(QString)),this,SLOT(setCurrentFile(QString)));
    childview->setDragMode(QGraphicsView::RubberBandDrag);

    // ���������� �������
    mdiArea->addSubWindow(childview);
    return childview;
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ������������ �������� �������
SubWindow *MainWindow::activeGraphicsView()
{

    ////qDebug() << "activeGraphicsView!!!!!!!!!";
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
    {
        return qobject_cast<SubWindow *>(activeSubWindow->widget());
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ��������� ���������  �������
// ������� ������:
//      window - ����, ������� ��������� ������� ��������
void MainWindow::setActiveSubWindow(QMdiSubWindow* window)
{
    this->statusBar()->showMessage("������� ������������");

    //qDebug() << "setActiveSubWindow!!!!!!!!!";
    if (!window || window == this->lastActiveWindow)
        return;
    
    mdiArea->setActiveSubWindow(window);
    lastActiveWindow = window;
    
    // ��������������� ����������� �����
    scene = 0;
    scene = qobject_cast<GraphScene *>(activeGraphicsView()->scene);
    for(int i = 0 ; i < sceneScaleCombo->count(); i++)
        if(this->sceneScaleCombo->itemText(i) == scene->getScale())
        {
        this->sceneScaleCombo->setCurrentIndex(i);
        break;
    }

    updateEditToolBar();
    
    // ���������� �����, �������
    scene->setMode(GraphScene::MoveMode);
    foreach(Vertex* item, scene->allVertexes)
    {
        addVertexImMatrix(item);
    }
    foreach(QGraphicsItem* item, scene->items())
    {
        if (item->type() == Edge::Type)
            addEdgeInMatrix(qgraphicsitem_cast<Edge *>(item));
    }
    if (!scene->answerList.isEmpty())
    {
        if (scene->answerList.count() / 1000 > 1)
            rangAction->setVisible(true);
        foundName->setMaximumHeight(50);
        gamiltonCombo->setMaximumHeight(50);
        // gamiltonCombo->addItems(scene->answerList);
        
        this->showResults(scene->rang);
        foundType->setText(scene->foundType);
        foundType->setMaximumHeight(50);
        
        foundedCount->setText(scene->foundCount);
        foundedCount->setMaximumHeight(50);
        
        foundTime->setText(scene->foundTime);
        foundTime->setMaximumHeight(50);
    }
}   
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� �������� �������
void MainWindow::updateEditToolBar()
{
    //qDebug() << "updateEditToolBar!!!!!!!!!";
    
    addEdgeAction->setChecked(false);
    addArcAction->setChecked(false);
    //sceneScaleCombo->setCurrentIndex(2);
    rangAction->setVisible(false);
    this->viewToolBal->hide();
    foundName->setMaximumHeight(0);
    gamiltonCombo->setMaximumHeight(0);
    gamiltonCombo->clear();
    foundType->setMaximumHeight(0);
    foundedCount->setMaximumHeight(0);
    foundTime->setMaximumHeight(0);
    prB->setValue(0);
    prB->hide();
    
    table->clear();
    table->setColumnCount(0);
    table->setRowCount(0);
    tableLabels.clear();

    //    scene->answerList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ��������� ����������� �������
void MainWindow::arrowReverse()
{
    this->statusBar()->showMessage("����������� ����� ���� ��������");

    if(scene->selectedItems().isEmpty())
        return;
    // ��������� �����
    QGraphicsItem *item =  scene->selectedItems().first();
    Edge *edge = qgraphicsitem_cast<Edge *>(item);
    if(edge == 0)
        return;
    
    // ��������� ���������/�������� ������� �����
    Vertex *itm;
    itm = edge->startItem();
    edge->setStartItem(edge->endItem());
    edge->setEndItem(itm);
    
    addEdgeInMatrix(edge);
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ��������� �������� �����
// ������� ������:
//      scale - ����� �������
void MainWindow::sceneScaleChanged(const QString &scale)
{
    if (scale == "")
        return;
    this->statusBar()->showMessage("������� ����� ���������");
    scene->setScale(scale);

    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QMatrix oldMatrix = activeGraphicsView()->matrix();
    activeGraphicsView()->resetMatrix();
    activeGraphicsView()->translate(oldMatrix.dx(), oldMatrix.dy());
    activeGraphicsView()->scale(newScale, newScale);
}
////////////////////////////////////////////////////////////////////////////////////////
// ���������� ������� ������ "�������� ����"
void MainWindow::addArc()
{
    this->statusBar()->showMessage("������������ ����������� ���������� ���");

    //qDebug() << "addArc";
    addEdgeAction->setChecked(false);
    if (addArcAction->isChecked())
        scene->setMode(GraphScene::AddArc);
    else
        scene->setMode(GraphScene::MoveMode);
    
}
////////////////////////////////////////////////////////////////////////////////////////
// ���������� ������� ������ "�������� �����"
void MainWindow::addEdge()
{
    this->statusBar()->showMessage("������������ ����������� ���������� �����");

    //qDebug() << "addEdge";
    addArcAction->setChecked(false);
    if (addEdgeAction->isChecked())
        scene->setMode(GraphScene::AddEdge);
    else
        scene->setMode(GraphScene::MoveMode);
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ������� ����� � �������
void MainWindow::clearScene()
{
    this->statusBar()->showMessage("����� �������");

    scene->clear();
    scene->allVertexes.clear();
    scene->setVertexCount(1);
    scene->setEdgeCount(1);
    table->clear();
    table->setColumnCount(0);
    table->setRowCount(0);
    tableLabels.clear();
    scene->answerList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////
// ���������� � ���������
void MainWindow::about()
{
    this->statusBar()->showMessage("�������� ������� � ���������");
    QMessageBox::about(this, "� ���������...","����������� ������ ������ ������������� ������.");
}
////////////////////////////////////////////////////////////////////////////////////////
// �������� �������� � �������� ������� � ��������
void MainWindow::createActions()
{
    deleteAction = new QAction(QIcon(":/images/delete.png"),"������� �������", this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setToolTip("������� ������� �� �����");
    connect(deleteAction, SIGNAL(triggered()),this, SLOT(deleteItem()));
    
    exitAction = new QAction(QIcon(":/images/exit.png"),"�����", this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setToolTip("����� �� ���������");
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    
    aboutAction = new QAction(QIcon(":/images/about.png"),"� ���������...", this);
    aboutAction->setShortcut(tr("Ctrl+B"));
    aboutAction->setToolTip("������� � ���������");
    connect(aboutAction, SIGNAL(triggered()),this, SLOT(about()));
    
    reverseAction = new QAction(QIcon(":/images/refresh.png"),"�������� �����������", this);
    reverseAction->setShortcut(tr("Ctrl+R"));
    reverseAction->setToolTip("�������� ����������� �������");
    connect(reverseAction, SIGNAL(triggered()),this, SLOT(arrowReverse()));
    
    exportAction = new QAction(QIcon(":/images/jpg.png"),"��������� ��� ��������", this);
    exportAction->setToolTip("��������� ���� ��� �����������");
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportImage()));
    
    newAction = new QAction(QIcon(":/images/new.png"),"������� ����� ����", this);
    newAction->setShortcut(tr("Ctrl+n"));
    newAction->setToolTip("������� ����� ����");
    connect(newAction, SIGNAL(triggered()),this, SLOT(newFile()));
    
    loadAction = new QAction(QIcon(":/images/open.png"),"���������", this);
    loadAction->setShortcut(tr("Ctrl+o"));
    loadAction->setToolTip("��������� ���� �� �����");
    connect(loadAction, SIGNAL(triggered()),this, SLOT(getFileName()));
    
    saveAction = new QAction(QIcon(":/images/save.png"),"���������", this);
    saveAction->setShortcut(tr("Ctrl+s"));
    saveAction->setToolTip("��������� ���� � ����");
    connect(saveAction, SIGNAL(triggered()),this, SLOT(save()));
    
    saveAsAction = new QAction(QIcon(":/images/saveas.png"),"��������� ���...", this);
    saveAsAction->setShortcut(tr("Ctrl+Shift+s"));
    saveAsAction->setToolTip("��������� ���...");
    connect(saveAsAction, SIGNAL(triggered()),this, SLOT(saveAs()));
    
    addEdgeAction = new QAction(QIcon(":/images/edge.png"),"�������� �����", this);
    addEdgeAction->setCheckable(true);
    addEdgeAction->setToolTip("�������� �����");
    connect(addEdgeAction, SIGNAL(triggered()),this, SLOT(addEdge()));

    addArcAction = new QAction(QIcon(":/images/arc.png"),"�������� ����", this);
    addArcAction->setCheckable(true);
    addArcAction->setToolTip("�������� ����");
    connect(addArcAction, SIGNAL(triggered()),this, SLOT(addArc()));

    findGamiltonAction = new QAction(QIcon(":/images/fg.png"),"����� ������������ �����", this);
    findGamiltonAction->setShortcut(tr("Ctrl+f"));
    findGamiltonAction->setToolTip("����� ������������ �����");
    connect(findGamiltonAction, SIGNAL(triggered()), this, SLOT(findGamilton()));
    
    setMatrixAction = new QAction(QIcon(":/images/number.png"),"���������� ������ �������", this);
    setMatrixAction->setToolTip("���������� ������ �������");
    connect(setMatrixAction, SIGNAL(triggered()), this, SLOT(setMatrixSize()));
    
//    buildGraphAction = new QAction(QIcon(":/images/make.png"),"��������� ���� �� �������", this);
//    buildGraphAction->setToolTip("��������� ���� �� �������");
//    connect(buildGraphAction, SIGNAL(triggered()), this, SLOT(buildGraph()));
    
    tileAct = new QAction(QIcon(":/images/tile.png"),"������������ �������", this);
    setMatrixAction->setToolTip("������������ �������");
    connect(tileAct, SIGNAL(triggered()), this, SLOT(tileSubWindows()));
    
    cascadeAct = new QAction(QIcon(":/images/cascade.png"),"������������ ��������", this);
    setMatrixAction->setToolTip("������������ ��������");
    connect(cascadeAct, SIGNAL(triggered()), this, SLOT(cascadeSubWindows()));
    
    closeAllSubAction = new QAction(QIcon(":/images/close.png"),"������� ��� �������", this);
    setMatrixAction->setToolTip("������� ��� �������");
    connect(closeAllSubAction, SIGNAL(triggered()), this, SLOT(closeAllSub()));
    
    saveInTxtAction = new QAction(QIcon(":/images/txt.png"),"��������� ���������� � ��������� ����", this);
    saveInTxtAction->setToolTip("��������� ���������� � ��������� ����");
    connect(saveInTxtAction, SIGNAL(triggered()),this, SLOT(saveInTxt()));
    
    printAction = new QAction(QIcon(":/images/print.png"),"������", this);
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, SIGNAL(triggered()), this, SLOT(filePrint()));
    
    previewAction = new QAction(QIcon(":/images/preview.png"),"��������������� ��������", this);
    connect(previewAction, SIGNAL(triggered()), this, SLOT(filePrintPreview()));
    
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }
    
    rangAction = new QAction(QIcon(":/images/list.png"),"�������� ���� �������", this);
    connect(rangAction, SIGNAL(triggered()), this, SLOT(rangChanged()));

    this->selectAllAction = new QAction(QIcon(":/images/select.png"),"�������� ���", this);
    selectAllAction->setShortcut(tr("Ctrl+a"));
    selectAllAction->setToolTip("�������� ��� �������");
    connect(selectAllAction, SIGNAL(triggered()),this, SLOT(selectAll()));


    prevAct = new QAction(QIcon(":/images/previous.png"),"���������� ���", this);
    prevAct->setToolTip("���������� ���");
    connect(prevAct, SIGNAL(triggered()),this, SLOT(prevStep()));

    startAct = new QAction(QIcon(":/images/start.png"),"��������� �����", this);
    startAct->setToolTip("��������� �����");
    connect(startAct, SIGNAL(triggered()),this, SLOT(startStep()));

    pauseAction = new QAction(QIcon(":/images/pause.png"),"������������� �����", this);
    pauseAction->setToolTip("������������� �����");
    connect(pauseAction, SIGNAL(triggered()),this, SLOT(pauseStep()));

    restartAct = new QAction(QIcon(":/images/repeat.png"),"���������� �����", this);
    restartAct->setToolTip("������������� �����");
    connect(restartAct, SIGNAL(triggered()),this, SLOT(repeatStep()));

    nextAct = new QAction(QIcon(":/images/next.png"),"��������� ���", this);
    nextAct->setToolTip("��������� ���");
    connect(nextAct, SIGNAL(triggered()),this, SLOT(nextStep()));
}

void MainWindow::selectAll()
{
    this->statusBar()->showMessage("��� ������� �� ����� ��������");

    qDebug() << "selectAll";

    foreach(QGraphicsItem *item, scene->items())
        item->setSelected(true);
}
void MainWindow::setMoveMode()
{
    this->statusBar()->showMessage("���������� ����� ����������� ��������");

    this->addEdgeAction->setChecked(false);
    this->addArcAction->setChecked(false);
    this->scene->setMode(GraphScene::MoveMode);
}

void MainWindow::closeAllSub()
{
    this->statusBar()->showMessage("��� ������� ���� �������");

    foreach(QMdiSubWindow* subW, mdiArea->subWindowList())
        subW->close();
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� ��������� ���������������� �������
void MainWindow::rangChanged()
{
    bool isText;
    QString text = QInputDialog::getText(this,"����� �������","������� ����� ������:",QLineEdit::Normal,"",&isText);
    
    if (text == "" || isText == false)
        return;
    // qDebug() << "comboRang";
    int rang = text.toInt();
    //  qDebug() << "rang  " << rang;
    
    if (rang < 0 || rang > scene->answerList.count() / 1000)
    {
        QMessageBox::warning(this, "������ ������ �������",
                             "������ ������� �������� ������ 1000 ��������� ������.\n"+
                             (QString)"��� ��������� ���� ����������� ������� ����� �� 0 �� "+
                             QString::number(scene->answerList.count()/1000));
        return;
    }
    
    scene->rang = rang;
    showResults(scene->rang);
}

void MainWindow::showResults(int rang)
{
    int number;
    qDebug() << "rang  " << rang;
    qDebug() << "scene->answerList.count() / 1000  " << scene->answerList.count() / 1000;
    
    if (scene->answerList.count() / 1000 <= rang)
    {
        qDebug() << "true  ";
        number = scene->answerList.count()-rang*1000;
    }
    else
        number = 1000;
    
    qDebug() << "number  " << number;
    
    QStringList results;
    results.clear();
    
    for (int i = rang*1000 ; i < rang*1000 + number ; i++)
    {
        QString row;
        for (int j = 0 ; j < scene->answerList[i].length() ; j++)
        {
            QVariant tmp(scene->answerList[i][j]);
            int value = tmp.toInt()-64;
            row += "v" + QString::number(value);
        }
        results << QString::number(i+1)+". "+row;
    }
    
    if (!scene->answerList.isEmpty())
    {
        gamiltonCombo->clear();
        
        statusBar()->showMessage("�������� ���� �������");
        gamiltonCombo->addItems(results);
        results.clear();
        
        gamiltonCombo->setFont(QFont("Arial",8,QFont::Bold));
        //gamiltonCombo->setCurrentIndex(-1);
        gamiltonCombo->setIconSize(QSize(40,40));
        gamiltonCombo->setToolTip("������������ �����");
        gamiltonCombo->setCurrentIndex(0);
    }
}
////////////////////////////////////////////////////////////////////////////////////////
// �������� ���� ���������
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu("����");
    fileMenu->addAction(newAction);
    fileMenu->addAction(loadAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exportAction);
    fileMenu->addAction(saveInTxtAction);
    fileMenu->addAction(previewAction);
    fileMenu->addAction(printAction);
    fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActs[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    redactionMenu = menuBar()->addMenu("��������������");
    redactionMenu->addAction(this->addArcAction);
    redactionMenu->addAction(this->addEdgeAction);
    redactionMenu->addAction(this->selectAllAction);
    redactionMenu->addAction(this->deleteAction);
    redactionMenu->addSeparator();
    redactionMenu->addAction(this->setMatrixAction);
//    redactionMenu->addAction(this->buildGraphAction);
    
    QMenu *findMenu = this->menuBar()->addMenu("�����");
    QAction *a;
    findMenu->addAction(this->findGamiltonAction);
    findMenu->addSeparator();
    a = new QAction("�������������� �����", this);
    connect(a, SIGNAL(triggered()), this, SLOT(algMet()));
    findMenu->addAction(a);
    a = new QAction("�������������� ���������� �����", this);
    connect(a, SIGNAL(triggered()), this, SLOT(algMetUpg()));
    findMenu->addAction(a);
    a = new QAction("����� ��������-�������", this);
    connect(a, SIGNAL(triggered()), this, SLOT(algRF()));
    findMenu->addAction(a);
    a = new QAction("����� ��������-������� ����������", this);
    connect(a, SIGNAL(triggered()), this, SLOT(algRFUpg()));
    findMenu->addAction(a);
    a = new QAction("������������ �����", this);
    connect(a, SIGNAL(triggered()), this, SLOT(algMultiChain()));
    findMenu->addAction(a);
    
    windowMenu = this->menuBar()->addMenu("����");
    windowMenu->addAction(this->tileAct);
    windowMenu->addAction(this->cascadeAct);
    windowMenu->addSeparator();
    windowMenu->addAction(this->closeAllSubAction);
    updateWindowMenu();
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));
    
    itemMenu = new QMenu("���� �������");
    itemMenu->addAction(deleteAction);
    
    arrowMenu = new QMenu("���� �����");
    arrowMenu->addAction(deleteAction);
    arrowMenu->addAction(reverseAction);
    
    aboutMenu = menuBar()->addMenu("�������");
    aboutMenu->addAction(aboutAction);
}

void MainWindow::updateWindowMenu()
{
    windowMenu->clear();
    windowMenu->addAction(this->tileAct);
    windowMenu->addAction(this->cascadeAct);

    windowMenu->addSeparator();

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    //    separatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        SubWindow *child = qobject_cast<SubWindow *>(windows.at(i)->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                   .arg(child->windowTitle());
        } else {
            text = tr("%1 %2").arg(i + 1)
                   .arg(child->windowTitle());
        }
        QAction *action  = windowMenu->addAction(text);
        action->setCheckable(true);
        action ->setChecked(child == this->activeGraphicsView());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windows.at(i));
    }
    windowMenu->addSeparator();
    windowMenu->addAction(this->closeAllSubAction);
}

void MainWindow::algMet()
{this->beginFind(0);}
void MainWindow::algMetUpg()
{this->beginFind(1);}
void MainWindow::algRF()
{this->beginFind(2);}
void MainWindow::algRFUpg()
{this->beginFind(3);}
void MainWindow::algMultiChain()
{this->beginFind(4);}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� �������� ���������
void MainWindow::createToolbars()
{
    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%");
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setIconSize(QSize(40,40));
    sceneScaleCombo->setCurrentIndex(2);
    sceneScaleCombo->setToolTip("������� �����");
    for(int i = 0; i < 8 ; i++) sceneScaleCombo->setItemIcon(i,QIcon(":/images/scale.png"));
    connect(sceneScaleCombo, SIGNAL(currentIndexChanged(QString)),this, SLOT(sceneScaleChanged(QString)));
    
    gamiltonCombo = new QComboBox;
    gamiltonCombo->setMaximumWidth(200);
    gamiltonCombo->setMaximumHeight(0);
    connect(gamiltonCombo, SIGNAL(activated(QString)),this, SLOT(gamiltonComboChanged(QString)));
    connect(gamiltonCombo, SIGNAL(currentIndexChanged(QString)),this, SLOT(gamiltonComboChanged(QString)));


    fileToolBar = addToolBar("������ � ������");   
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(loadAction);
    fileToolBar->addAction(saveAction);
    fileToolBar->addAction(saveAsAction);
    fileToolBar->addSeparator();
    fileToolBar->addAction(exportAction);
    fileToolBar->addAction(saveInTxtAction);
    fileToolBar->addAction(previewAction);
    fileToolBar->addAction(printAction);
    fileToolBar->addSeparator();
    fileToolBar->addWidget(sceneScaleCombo);
    fileToolBar->addAction(tileAct);
    fileToolBar->addAction(cascadeAct);
    
    fileToolBar->setIconSize(QSize(40,40));
    
    table = new QTableWidget(0,0,this);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFixedSize(200,200);
    connect(table,SIGNAL(cellChanged(int,int)),this,SLOT(cellChange(int,int)));
    connect(table,SIGNAL(cellClicked(int,int)),this,SLOT(cellClicked(int,int)));

    QLabel *label = new QLabel("������� ���������");
    label->setFont(QFont("Arial",8,QFont::Bold));
    label->setAlignment(Qt::AlignCenter);
    
    foundName = new QLabel("��������� ����������");
    foundName->setFont(QFont("Arial",8,QFont::Bold));
    foundName->setAlignment(Qt::AlignCenter);
    foundName->setMaximumHeight(0);
    
    foundType= new QLabel("");
    foundType->setMaximumHeight(0);
    foundType->setFont(QFont("Arial",8,QFont::Normal,true));
    
    foundedCount= new QLabel("");
    foundedCount->setMaximumHeight(0);
    foundedCount->setFont(QFont("Arial",8,QFont::Normal,true));
    
    foundTime= new QLabel("");
    foundTime->setMaximumHeight(0);
    foundTime->setFont(QFont("Arial",8,QFont::Normal,true));
    
    prB= new QProgressBar;
    prB->setMinimum(0);
    prB->setMaximum(100);
    prB->setValue(0);
    prB->setMaximumWidth(200);

    QAction *startVis = new QAction(QIcon(":/images/vis.png"),"������������ ������",this);
    startVis->setCheckable(true);
    connect(startVis,SIGNAL(triggered()),this,SLOT(startVisual()));
    
    editToolBar = addToolBar("��������������");
    addToolBar(Qt::RightToolBarArea,editToolBar);
    editToolBar->addAction(addArcAction);
    editToolBar->addAction(addEdgeAction);
    editToolBar->addAction(deleteAction);
    editToolBar->addSeparator();
    editToolBar->addAction(findGamiltonAction);
    editToolBar->addAction(startVis);
    editToolBar->addWidget(foundName);
    editToolBar->addWidget(gamiltonCombo);
    editToolBar->addAction(rangAction);
    editToolBar->addWidget(label);
    editToolBar->addWidget(table);
    editToolBar->addAction(setMatrixAction);
//    editToolBar->addAction(buildGraphAction);
    editToolBar->addSeparator();
    editToolBar->addWidget(foundType);
    editToolBar->addWidget(foundedCount);
    editToolBar->addWidget(foundTime);
    editToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    editToolBar->setIconSize(QSize(40,40));
    
    rangAction->setVisible(false);
    
    statusBar()->showMessage("�����");
    statusBar()->addPermanentWidget(prB);
    prB->hide();


    viewToolBal = addToolBar("��������");
    addToolBar(Qt::BottomToolBarArea,viewToolBal);

    viewToolBal->addAction(prevAct);
    viewToolBal->addAction(nextAct);
    viewToolBal->addAction(startAct);

    spin = new QSpinBox;
    spin->setMinimum(10);
    spin->setMaximum(10000);
    spin->setSingleStep(100);
    spin->setValue(500);
    viewToolBal->addWidget(spin);
    QLabel *l = new QLabel("��");
    viewToolBal->addWidget(l);

    viewToolBal->addAction(pauseAction);
    viewToolBal->addAction(restartAct);

    pathLabel = new QLabel;
    pathLabel->setMinimumWidth(400);
    viewToolBal->addWidget(pathLabel);
    viewToolBal->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    viewToolBal->setIconSize(QSize(40,40));
    this->viewToolBal->hide();
}

void MainWindow::repeatStep()
{
    this->statusBar()->showMessage("����� ����������");
    this->timer->stop();
    this->scene->clearSelection();
    this->viewer->counter = 0;
    this->viewer->path.clear();
    this->viewer->paths.clear();
    this->prevStep();
    this->pathLabel->setText("");
    this->nextAct->setEnabled(true);
}

void MainWindow::pauseStep()
{
    this->statusBar()->showMessage("����� �������������");

    this->pauseAction->setDisabled(true);
    this->startAct->setEnabled(true);
    this->timer->stop();
    this->prevAct->setEnabled(true);
    this->nextAct->setEnabled(true);
}

void MainWindow::startStep()
{
    this->statusBar()->showMessage("����� �������");

    timer->start(spin->value());
    this->pauseAction->setEnabled(true);
    this->restartAct->setEnabled(true);
    this->startAct->setDisabled(true);
    this->prevAct->setDisabled(true);
    this->nextAct->setDisabled(true);
}

void MainWindow::timeout()
{
    this->nextStep();
    if (this->viewToolBal->isVisible())
        timer->start(spin->value());
}
void MainWindow::prevStep()
{
    qDebug( ) << "prevStep " ;
    this->prevAct->setDisabled(true);
    this->restartAct->setDisabled(true);
    viewer->doPrev();
}

void MainWindow::nextStep()
{
    qDebug( ) << "nextStep " ;
    viewer->doNext();
    this->prevAct->setEnabled(true);
    this->restartAct->setEnabled(true);
}

void MainWindow::startVisual()
{
    if (this->viewToolBal->isVisible())
    {
        this->statusBar()->showMessage("������������ �����������");
        this->scene->clearSelection();
        this->viewToolBal->hide();
        viewer->~Viewer();
        timer->~QTimer();
    }
    else
    {
        if(scene->items().isEmpty())
        {
            QMessageBox::warning(this,"��������� ������ ������������� ������","�������� �������� �� ����������� �����");
            return;
        }
        this->scene->clearSelection();

        int tableSize = table->columnCount();
        QList<QStringList> Mmatrix;
        QStringList curColumn;
        for (int i = 0 ; i < tableSize ; i++)
        {
            for (int j = 0 ; j < tableSize ; j++)
            {
                if (table->item(i,j)->text() == "1" && i != j)
                {
                    curColumn << tableLabels[j];
                }
            }
            Mmatrix << curColumn;
            curColumn.clear();
        }

        qDebug() << "----------begin visual------------------";
        viewer = new Viewer(Mmatrix,tableLabels,statusBar());
        Mmatrix.clear();
        connect(viewer,SIGNAL(finish(QList<QStringList>)),this,SLOT(finish(QList<QStringList>)));
        connect(viewer,SIGNAL(pathUpdate(QStringList,int )),this,SLOT(pathUpdate(QStringList,int)));
        qDebug() << "curS first "<< viewer->curS;

        this->viewToolBal->show();
        this->pauseAction->setDisabled(true);
        this->prevAct->setDisabled(true);
        this->restartAct->setDisabled(true);
        this->startAct->setEnabled(true);
        this->pathLabel->setText("");

        timer = new QTimer;
        connect(timer,SIGNAL(timeout()),this,SLOT(timeout()));
    }
}

void MainWindow::finish(QList<QStringList> paths)
{
    this->scene->clearSelection();
    this->viewToolBal->hide();
    viewer->~Viewer();
    timer->~QTimer();

    gamiltonCombo->clear();
    scene->foundType="";
    scene->foundCount="";
    scene->foundTime="";

    QStringList answers;
    for (int i = 0 ; i < paths.size() ; i++)
    {
        QString s = "";
        for (int j = 0 ; j < paths[i].size() ; j++)
        {
            int value = paths[i][j].remove(0,1).toInt();
            s+= QString(64+value);
        }
        answers << s;
    }
    scene->answerList.clear();
    scene->answerList = answers;

    if (!scene->answerList.isEmpty())
    {
        statusBar()->showMessage("������������ ����� �������");

        foundName->setMaximumHeight(50);
        if (scene->answerList.count() / 1000 > 1)
            rangAction->setVisible(true);

        gamiltonCombo->setMaximumHeight(50);
        this->showResults(0);

        scene->foundType = "������������ ������";
        foundType->setText(scene->foundType);
        foundType->setMaximumHeight(50);

        int answerCount = scene->answerList.count();
        scene->foundCount = "���������� ��������� ������: "+QString::number(answerCount);
        foundedCount->setText(scene->foundCount);
        foundedCount->setMaximumHeight(50);

        scene->foundTime = "����� ������: ������������";
        foundTime->setText(scene->foundTime);
        foundTime->setMaximumHeight(50);
    }
    else
    {
        QMessageBox::warning(this,"��������� ������ ������������� ������","������������ ����� �� �������",QMessageBox::Ok);
        this->statusBar()->showMessage("������������ ����� �� �������");
    }
}

void MainWindow::pathUpdate(QStringList path, int countCycles)
{
    this->prevAct->setEnabled(true);
    this->restartAct->setEnabled(true);
    scene->clearSelection();
    for (int i = 0 ; i < path.size() ; i++)
    {
        foreach(Vertex *v, scene->allVertexes)
        {
            if(v->getName() == path[i])
            {
                v->setSelected(true);
                foreach(Edge* e, v->edges)
                    if(i < path.size()-1 && (e->endItem()->getName() == path[i+1] ||
                                             (e->startItem()->getName() == path[i+1] && e->endItem()->getName() == v->getName() && e->arrowType() == Edge::EdgeType)))
                        e->setSelected(true);
                break;
            }
        }
    }
    QString outPath,cycles;
    if (countCycles)
        cycles = "���-�� ������: " + QString::number(countCycles) + "    ";
    for (int i = 0 ; i < path.size() ; i++)
        outPath += path[i];

    if (outPath!= "")
        pathLabel->setText(cycles + "������� ����: " + outPath);
    else
    {
        pathLabel->setText("");
        this->prevAct->setDisabled(true);
        this->restartAct->setDisabled(true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// ������������ ������� �������
void MainWindow::tileSubWindows()
{
    this->statusBar()->showMessage("������� ����������� �������");

    mdiArea->tileSubWindows();
    foreach(QMdiSubWindow* subW,mdiArea->subWindowList())
    {
        SubWindow *sW = qobject_cast<SubWindow *>(subW->widget());
        sW->centerScrollBar();
    }
}
////////////////////////////////////////////////////////////////////////////////////////
// ������������ ������� ��������
void MainWindow::cascadeSubWindows()
{
    this->statusBar()->showMessage("������� ����������� ��������");

    mdiArea->cascadeSubWindows();
    foreach(QMdiSubWindow* subW,mdiArea->subWindowList())
    {
        SubWindow *sW = qobject_cast<SubWindow *>(subW->widget());
        sW->centerScrollBar();
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// ��������� �������� ����� �����
// ������� ������:
//      fileName - ��� �����
void MainWindow::setCurrentFile(const QString &fileName)
{
    qDebug() << "setCurrentFile";
    setWindowFilePath(fileName);
    
    QSettings settings("OrganizationName", "ApplicationName");
    QStringList files = settings.value("recentFileList").toStringList();
    for (int i = 0 ; i < files.count() ; i++)
    {
        qDebug() << files[i];
    }
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    
    settings.setValue("recentFileList", files);
    for (int i = 0 ; i < files.count() ; i++)
    {
        qDebug() << i  << " " << files[i];
    }
    this->updateRecentFileActions();
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� ��������� �����
void MainWindow::openRecentFile()
{
    qDebug() << "openRecentFile";
    
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        QString fileName = action->data().toString();
        this->load(fileName);
    }
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� ������ ������� �������� ������
void MainWindow::updateRecentFileActions()
{
    qDebug() << "updateRecentFileActions";
    
    QSettings settings("OrganizationName", "ApplicationName");
    QStringList files = settings.value("recentFileList").toStringList();
    
    for (int i = 0 ; i < files.count() ; i++)
    {
        qDebug() << files[i];
    }
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
    
    qDebug() << "numRecentFiles" << numRecentFiles;
    
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1. %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        qDebug() << "text " << text;
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
    
    ///separatorAct->setVisible(numRecentFiles > 0);
}

////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� ����� � �����������
void MainWindow::exportImage()
{
    this->statusBar()->showMessage("���������� ���������");

    QFileDialog::Options options;
    options = 0;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,"��������� ��������",".png",tr("Png (*.png);;Pdf (*.pdf)"));
    if (!fileName.isEmpty()){
        if(selectedFilter=="Pdf (*.pdf)")
        {
            QRectF rect=scene->itemsBoundingRect();
            rect.adjust(-10,-10,10,10);
            QPrinter printer;
            printer.setOutputFileName(fileName);
            QSizeF size=printer.paperSize(QPrinter::Millimeter);
            size.setHeight(size.width()*rect.height()/rect.width());
            printer.setPaperSize(size,QPrinter::Millimeter);
            printer.setPageMargins(0,0,0,0,QPrinter::Millimeter);
            QPainter painter(&printer);
            painter.setRenderHint(QPainter::Antialiasing);
            scene->render(&painter,QRectF(),rect);
        }
        else {
            
            getPixmap().save(fileName);
        }        
    }
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������������� ���������� ������� � �������
// ������� ������:
//      vertex - ����������� �������
void MainWindow::addVertexImMatrix(Vertex* vertex)
{

    table->setColumnCount(table->columnCount()+1);
    table->setRowCount(table->rowCount()+1);
    
    int tableSize = table->columnCount();
    //qDebug() << "tableSize " << tableSize;
    for (int i = 0 ; i < tableSize ; i++)
        for (int j = 0 ; j < tableSize ; j++)
            if (i == tableSize - 1 || j == tableSize-1)
            {
        QTableWidgetItem *cell = new QTableWidgetItem("0");
        table->setItem(i,j,cell);
    }
    tableLabels.append(vertex->getName());
    resizeTable();
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������������� ���������� ����� � �������
// ������� ������:
//      edge - ����������� �����
void MainWindow::addEdgeInMatrix(Edge* edge)
{
    int tableSize = table->columnCount();
    //qDebug() << "tableSize " << tableSize;
    QString startName = edge->startItem()->getName();
    QString endName = edge->endItem()->getName();
    int startPos = -1,endPos = -1;
    
    for (int i = 0 ; i < tableSize ; i++)
    {
        if (table->horizontalHeaderItem(i)->text() == startName)
            startPos = i;
        if (table->horizontalHeaderItem(i)->text() == endName)
            endPos = i;
    }
    
    //qDebug() << "startPos " << startPos;
    //qDebug() << "endPos " << endPos;
    
    if (startPos != -1 && endPos != -1)
    {
        table->item(endPos,startPos)->setText("0");
        table->item(startPos,endPos)->setText("1");
        if (edge->arrowType() == Edge::EdgeType)
            table->item(endPos,startPos)->setText("1");
    }
    resizeTable();
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� �������� � ����������� �����
void MainWindow::deleteItem()
{
    if (this->scene->selectedItems().isEmpty())
        return;
    foreach (QGraphicsItem *item, scene->selectedItems())
    {
        // �������� �������
        if (item->type() == Vertex::Type)
        {
            Vertex *curItem = qgraphicsitem_cast<Vertex *>(item);
            // �������� ������� �����
            foreach(Edge* edge, curItem->edges)
            {
                edge->endItem()->edges.removeOne(edge);
                scene->removeItem(edge);
            }
            // ���������� �������
            int tableSize = table->columnCount(),deletedCell = -1;
            //qDebug() << "tableSize " << tableSize;
            for (int i = 0 ; i < tableSize ; i++)
                if (table->horizontalHeaderItem(i)->text() == curItem->getName())
                {
                deletedCell = i;
                break;
            }
            
            if (deletedCell != -1)
            {
                table->removeColumn(deletedCell);
                table->removeRow(deletedCell);
                deletedCell = -1;
            }
            tableLabels.removeAt(tableLabels.indexOf(curItem->getName()));
            scene->allVertexes.removeOne(curItem);
            scene->setVertexCount(1);
            for (int i = 0 ; i < tableLabels.count() ; i++)
            {
                QString s = tableLabels[i];
                int nameValue = s.remove(0,1).toInt();
                if (scene->getVertexCount() <= nameValue)
                    scene->setVertexCount(nameValue+1);
            }
        }
        // �������� �����
        if (item->type() == Edge::Type)
        {
            Edge *edge = qgraphicsitem_cast<Edge *>(item);
            
            int tableSize = table->columnCount();
            //qDebug() << "tableSize " << tableSize;
            QString startName = edge->startItem()->getName();
            QString endName = edge->endItem()->getName();
            int startPos = -1,endPos = -1;
            
            for (int i = 0 ; i < tableSize ; i++)
            {
                if (table->horizontalHeaderItem(i)->text() == startName)
                    startPos = i;
                if (table->horizontalHeaderItem(i)->text() == endName)
                    endPos = i;
            }
            
            //qDebug() << "startPos " << startPos;
            //qDebug() << "endPos " << endPos;
            
            // ���������� �������
            if (startPos != -1 && endPos != -1)
            {
                table->item(startPos,endPos)->setText("0");
                table->item(endPos,startPos)->setText("0");
            }
            resizeTable();
            
            edge->startItem()->removeArrow(edge);
            edge->endItem()->removeArrow(edge);
        }
        
        scene->removeItem(item);
    }
    scene->setModified(true);
    this->clearResult();
    this->scene->clearSelection();
    resizeTable();
    this->statusBar()->showMessage("������ ������");

    if(scene->allVertexes.isEmpty())
    {
        //qDebug() <<"ffff";
        scene->setVertexCount(1);
        scene->setEdgeCount(1);
        this->statusBar()->showMessage("��� ���������� ������� �������� ��� ���� �� ����������� �����.");

    }
}

void MainWindow::clearResult()
{
    scene->clearSelection();
    scene->answerList.clear();
    gamiltonCombo->clear();
    scene->foundType="";
    scene->foundCount="";
    scene->foundTime="";

    foundName->setMaximumHeight(0);
    gamiltonCombo->setMaximumHeight(0);
    foundType->setMaximumHeight(0);
    foundedCount->setMaximumHeight(0);
    foundTime->setMaximumHeight(0);
    prB->hide();
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ������������ ������/������ �����
void MainWindow::resizeTable()
{
    table->setHorizontalHeaderLabels(tableLabels);
    table->setVerticalHeaderLabels(tableLabels);
    
    for (int i = 0 ; i < table->columnCount() ; i++)
    {
        table->setColumnWidth(i,20);
    }
    for (int i = 0 ; i < table->rowCount() ; i++)
    {
        table->setRowHeight(i,20);
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// ������� ������ ������ �������������� �����
void MainWindow::findGamilton()
{
    //    // ����� ������ ���������� ������������� ������
    //    QMessageBox msgBox(this);
    //    msgBox.setText("�������� �������� ������ ������������� ������");
    //    QPushButton *algebr = msgBox.addButton("�������������� �����", QMessageBox::ActionRole);
    //    msgBox.addAction(saveAction);
    //    QPushButton *algebrUpg = msgBox.addButton("�������������� �����.", QMessageBox::ActionRole);
    //    QPushButton *roberta = msgBox.addButton("����� ��������-�������", QMessageBox::ActionRole);
    //    QPushButton *robertaUpg = msgBox.addButton("�����. ����� ��������-�������", QMessageBox::ActionRole);
    //    QPushButton *abortButton = msgBox.addButton("������",QMessageBox::ActionRole);
    //
    //    msgBox.exec();
    //    int type;
    //    if (msgBox.clickedButton() == algebr)
    //        type = 0;
    //    else if (msgBox.clickedButton() == algebrUpg)
    //        type = 1;
    //    else if (msgBox.clickedButton() == roberta)
    //        type = 2;
    //    else if (msgBox.clickedButton() == robertaUpg)
    //        type = 3;
    //    else if (msgBox.clickedButton() == abortButton)
    //        return;
    //
    //    this->beginFind(type);

    int hideBut = 0;
    if (this->tableLabels.size() > 11)
    {
        if (this->tableLabels.size() > 20)
            hideBut = 3;
        else
        {
            int max = -1;
            for (int i = 0 ; i < this->tableLabels.size() ; i++)
            {
                int curC = this->getCountEdges(i);
                if (max < curC)
                    max = curC;
            }
            if (max <= 6)
                hideBut = 1;
            else
                hideBut = 2;
        }
    }

    qDebug () << "______________-----__________" << hideBut;

    Dialog *dlg = new Dialog(hideBut,this);
    dlg->exec();
}

void MainWindow::beginFind(int type)
{
    if(scene->items().isEmpty())
    {
        QMessageBox::warning(this,"��������� ������ ������������� ������","�������� �������� �� ����������� �����");
        return;
    }
    prB->setValue(0);
    int tableSize = table->columnCount();
    
    for (int k = 0; k < tableSize; k++)
    {
        bool in = false, out = false;
        
        for (int i = 0; i < tableSize; i++)
        {
            if (table->item(i,k)->text()=="1")
                in = true;
            if (table->item(k,i)->text()=="1")
                out = true;
        }
        if (!in || !out)
        {
            QMessageBox::warning(this,"��������� ������ ������������� ������","������������ ����� �� �������");
            return;
        }        
    }
    QList<QStringList > matrix;
    QList<QList<int> > matrixM;

    if (type != 4)
    {
        QStringList row;
        // ���������� ������ �� �������
        for (int i = 0 ; i < tableSize ; i++)
        {
            for (int j = 0 ; j < tableSize ; j++)
            {
                row << table->item(i,j)->text();
            }
            matrix << row;
            row.clear();
        }
    }
    else
    {
        QList<int> row;
        // ���������� ������ �� �������
        for (int i = 0 ; i < this->tableLabels.count() ; i++)
        {
            for (int j = 0 ; j < tableLabels.count() ; j++)
            {
                row << table->item(i,j)->text().toInt();
            }
            matrixM << row;
            row.clear();
        }
    }
    // ���������� ���� ������ �� �������
    QStringList vertexNames;
    for (int i = 0 ; i < tableSize ; i++)
    {
        QString s = tableLabels[i];
        int value = s.remove(0,1).toInt();
        vertexNames << QString(64+value);
    }
    // �������� �����
    this->clearResult();

    QTime time;
    prB->show();
    statusBar()->showMessage("����� ������������� ������");
    // ������ ������ �������������� �����
    int algTime = time.elapsed();

    if (type!= 4)
    {
        qDebug() << "create graph";
        Graph *graph = new Graph(matrix, vertexNames, prB);
        time.start();
        qDebug() << "start find graph";

        scene->answerList = graph->findGamilton(type);
        algTime = time.elapsed();
        graph->~Graph();
    }
    else
    {
        CMultProcessor graph(matrixM, vertexNames, prB);
        time.start();
        scene->answerList = graph.ThreadMultiFind();
        algTime = time.elapsed();
        graph.~CMultProcessor();
    }

    prB->setValue(100);
    statusBar()->showMessage("������������ ����� �������");
    
    matrix.clear();
    matrixM.clear();
    vertexNames.clear();
    
    qDebug() << "algTime " << algTime;

    
    QStringList foundTypes;
    foundTypes << "�������������� �����" << "�������������� �����."
            << "����� ������c�-�������" << "�����. ����� ��������-�������" << "������������ �����" << "sdf";
    
    // ��������� �����������
    if (!scene->answerList.isEmpty())
    {
        //        for (int i = 0 ; i < scene->answerList.count() ; i++)
        //        {
        //            qDebug() << scene->answerList[i];
        //        }
        
        foundName->setMaximumHeight(50);
        if (scene->answerList.count() / 1000 > 1)
            rangAction->setVisible(true);
        
        gamiltonCombo->setMaximumHeight(50);
        this->showResults(0);
        
        scene->foundType = foundTypes[type];
        foundType->setText(scene->foundType);
        foundType->setMaximumHeight(50);
        
        int answerCount = scene->answerList.count();
        scene->foundCount = "���������� ��������� ������: "+QString::number(answerCount);
        foundedCount->setText(scene->foundCount);
        foundedCount->setMaximumHeight(50);
        
        scene->foundTime = "����� ������: "+QString::number(algTime) + "��";
        foundTime->setText(scene->foundTime);
        foundTime->setMaximumHeight(50);
    }
    else
    {
        QMessageBox::warning(this,"��������� ������ ������������� ������","������������ ����� �� �������",QMessageBox::Ok);

    }
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� ��������� �������� ����
// ������� ������:
//      s - ������� ����
void MainWindow::gamiltonComboChanged(const QString &s)
{
    qDebug() << "s  " << s;
    //    gamiltonCombo->setItemIcon(gamiltonCombo->currentIndex(),QIcon(":/images/circle.png"));
    scene->clearSelection();
    QString path=s;    
    path = path.remove(0,path.indexOf(".")+2);
    
    statusBar()->showMessage("��������� ����: " + path);
    
    foreach(QGraphicsItem* item, scene->items())
    {
        if (item->type() == Vertex::Type)
            item->setSelected(true);
    }
    QStringList vertexes;
    QString str;
    
    for (int i = 0 ; i < path.length(); i++)
    {
        if ((path[i] == 'v' && i!= 0) || (i == path.length()-1))
        {
            if (i == path.length()-1)
                str+=path[i];
            //qDebug() << "str " << str;
            vertexes << str;
            str = path[i];
        }
        else
            str+=path[i];
    }
    
    for (int i = 0 ; i < vertexes.count()-1; i++)
    {
        QString beg = vertexes[i],end = vertexes[i+1];
        foreach(QGraphicsItem* item, scene->items())
        {
            if (item->type() == Vertex::Type)
            {
                Vertex *curItem = qgraphicsitem_cast<Vertex *>(item);
                if(curItem->getName() == beg)
                {
                    foreach(Edge* edge, curItem->edges)
                        if ((edge->startItem() == curItem && edge->endItem()->getName() == end)
                            ||(edge->endItem() == curItem && edge->startItem()->getName() == end))
                            edge->setSelected(true);
                }
            }
        }
    }
    vertexes.clear();
    scene->setMoveBug(true);
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� ������ �������
void MainWindow::newFile()
{
    SubWindow *child = createGraphicsView();
    child->show();
    setActiveSubWindow(mdiArea->subWindowList().last());
    scene->setScale("100%");
    this->statusBar()->showMessage("����� ���� ������. ��� ���������� ������� �������� ��� ���� �� ����������� �����");
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� �����
void MainWindow::save()
{
    if (activeGraphicsView() && activeGraphicsView()->save())
        statusBar()->showMessage("���� ��������", 2000);
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� ����� � ����� ������
void MainWindow::saveAs()
{
    if (activeGraphicsView() && activeGraphicsView()->saveAs())
        statusBar()->showMessage("���� ��������", 2000);
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ��������� ����� ������������ �����
void MainWindow::getFileName()
{
    QString fileName =  QFileDialog::getOpenFileName(this,"������� ����",".grph","��� ���� (*.grph)");
    load(fileName);
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� �����
// ������� ������:
//      fileName - ��� ������������ �����
void MainWindow::load(QString fileName)
{
    if (fileName.isEmpty())
        return;
    
    if (mdiArea->subWindowList().count() == 0 || !scene->items().isEmpty())
    {
        newFile();
    }    
    if (activeGraphicsView()->maybeSave())
    {
        clearScene();
        updateEditToolBar();
        
        if (activeGraphicsView()->load(fileName))
            this->statusBar()->showMessage("���� ��������");
        this->activeGraphicsView()->centerScrollBar();
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� ����������� � ��������� ����
void MainWindow::saveInTxt()
{
    if (scene->answerList.isEmpty())
    {
        QMessageBox::warning(this,"������ ������","������ ����������� ����");
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this,"��������� ���...",".txt","��������� �������� (*.txt)");
    if (fileName.isEmpty())
        return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        QMessageBox::warning(this,"������ ���������� ������",file.errorString());
    else
    {
        QByteArray data;
        data.append("���������� ������ ������������� ������");
        data.append("\n--------------------------------------");
        data.append("\n����� ������: "+scene->foundType+"\n"+scene->foundCount+"\n"+scene->foundTime);
        data.append("\n--------------------------------------");
        for (int i = 0 ; i < scene->answerList.count() ; i++)
        {
            QString row;
            for (int j = 0 ; j < scene->answerList[i].length() ; j++)
            {
                QVariant tmp(scene->answerList[i][j]);
                int value = tmp.toInt()-64;
                row += "v" + QString::number(value);
            }
            data.append("\n"+QString::number(i+1)+". "+row);
        }
        
        file.write(data);
        this->statusBar()->showMessage("���������� ��������� � ��������� ����");
    }
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� �������� ����
void MainWindow::closeEvent(QCloseEvent *e)
{
    foreach(QMdiSubWindow* subW, mdiArea->subWindowList())
    {
        setActiveSubWindow(subW);
        if (activeGraphicsView()->maybeSave())
            e->accept();
        else
            e->ignore();
    }
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� ������� ������������� �������
void MainWindow::setMatrixSize()
{
    this->statusBar()->showMessage("��������� ������� �������");

    //qDebug() << "setMatrixSize ";
    bool bOk;
    int size = QInputDialog::getText(this,"�������� �������","������� ���������� ������(�� 1 �� *) ",QLineEdit::Normal,"",&bOk).toInt();
    if (size < 1 || size > 50)
    {
        QMessageBox::warning(this,"��������� ������ ������������� ������","���������� ������ ������ ���� � �������� �� 1 �� *\n���������� ������ ����������� � ������� 10");
        size = 10;
    }
    if (bOk == true && size > 0)
    {
        table->clear();
        tableLabels.clear();
        //qDebug() << "size " << size;
        table->setColumnCount(size);
        table->setRowCount(size);
        for (int i = 1; i < size+1 ; i++)
            tableLabels << "v"+QString::number(i);
        
        scene->setVertexCount(size+1);
        
        for (int i = 0 ; i < size; i++)
            for (int j = 0 ; j < size ; j++)
            {
            QTableWidgetItem *cell = new QTableWidgetItem("0");
            table->setItem(i,j,cell);
        }        
        resizeTable();
    }
    this->statusBar()->showMessage("������ ������� ����������");
buildGraph();
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������� ��������� ������ ������ �������
void MainWindow::cellChange(int i, int j)
{
    if (table->item(i,j)->text() != "0" && table->item(i,j)->text() != "1")
    {
        table->item(i,j)->setText("1");
        QMessageBox::warning(this,"��������� ������ ������������� ������",
                             "������� �������� ������ � ���������.\n���� ����� ��������� ���� �����, �� �������� 1, ����� �������� 0.");
    }
    if (table->item(i,j)->text() == "1")
    {
        if (i == j && tableLabels.count()!=1)
        {
            table->item(i,j)->setText("0");
        }
        int countEdges = this->getCountEdges(i);
        if (this->tableLabels.size() > 11 && countEdges > 10)
        {
            table->item(i,j)->setText("0");
            QMessageBox::warning(0,(QString)"���������� �����",
                                 (QString)"����� �� ����� ���� ���������, ��� ��� ���������� ��������/��������� ��� ������ ���� ������ 10");
        }
    }
    //qDebug() << "countEdge " << countEdge;
}

void MainWindow::cellClicked(int i,int j)
{
    if (i == j)
        return;

    if (this->table->item(i,j)->text() == "1")
    {
        if (this->table->item(j,i)->text() == "0")
        {
            foreach(Vertex* v, scene->allVertexes)
            {
                if(v->getName() == this->tableLabels[i])
                {
                    foreach(Edge* e, v->edges)
                    {
                        if(e->startItem() == v && e->endItem()->getName() == this->tableLabels[j])
                        {
                            scene->clearSelection();
                            e->setSelected(true);
                            scene->update();
                            break;
                        }
                    }
                    break;
                }
            }

            this->deleteItem();
        }
        else
        {
            foreach(Vertex* v, scene->allVertexes)
            {
                if(v->getName() == this->tableLabels[i])
                {
                    foreach(Edge* e, v->edges)
                    {
                        if(e->startItem() == v && e->endItem()->getName() == this->tableLabels[j])
                        {
                            e->setType(Edge::ArcType);
                            Vertex *v = e->startItem();
                            e->setStartItem(e->endItem());
                            e->setEndItem(v);
                            this->addEdgeInMatrix(e);
                            scene->update();
                            break;
                        }
                        else if(e->endItem() == v && e->startItem()->getName() == this->tableLabels[j])
                        {
                            e->setType(Edge::ArcType);
                            this->addEdgeInMatrix(e);
                            scene->update();
                            break;
                        }
                    }
                    break;
                }
            }

        }
    }
    //        this->table->item(i,j)->setText("0");
    else
    {
        if (this->table->item(j,i)->text() == "1")
        {
            foreach(Vertex* v, scene->allVertexes)
            {
                if(v->getName() == this->tableLabels[j])
                {
                    foreach(Edge* e, v->edges)
                    {
                        if(e->startItem() == v && e->endItem()->getName() == this->tableLabels[i])
                        {
                            e->setType(Edge::EdgeType);
                            this->addEdgeInMatrix(e);
                            scene->update();
                            break;
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            Vertex *stV, *eV;
            foreach(Vertex* v,scene->allVertexes)
            {
                if(v->getName() == this->tableLabels[i])
                    stV = v;
                if(v->getName() == this->tableLabels[j])
                    eV = v;
            }
            QString name = "e"+QString::number(scene->getEdgeCount());
            scene->setEdgeCount(scene->getEdgeCount()+1);
            Edge *e = scene->addEdge(name,stV,eV,Edge::ArcType);
            this->addEdgeInMatrix(e);
            scene->update();
        }
        //        this->table->item(i,j)->setText("1");
    }
}

int MainWindow::getCountEdges(int i)
{
    int tSize = table->columnCount();

    int countEdge=0;
    for (int k = 0; k <tSize; k++)
    {
        if (table->item(i,k)->text() == "1")
            countEdge++;
        if (table->item(k,i)->text() == "1")
            countEdge++;
    }
    qDebug() << " i ::  " << i << "  countEDge :: " << countEdge;
    return countEdge;
}

////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� ����� �� �������
void MainWindow::buildGraph()
{
    ////qDebug() << "buildGraph ";
    int fff=0;
    qDebug() << "build " << fff++;
    scene->clear();
    scene->allVertexes.clear();
    qDebug() << "build " << fff++;
    
    int tSize = table->columnCount();
    for (int i = 0 ; i < tSize; i++)
    {
        scene->addVertex(tableLabels[i]);
    }
    qDebug() << "build " << fff++;
    
    foreach (Vertex* vert,scene->allVertexes)
        qDebug() << vert->getName();
    
    for (int i = 0; i < tSize; i++)
    {
        double xCoord,yCoord,radius=300,rad;
        const qreal Pi = 3.14;
        rad = Pi/4 + 2*Pi*i/tSize;
        xCoord = radius*cos(rad);
        yCoord = -radius*sin(rad);
        scene->allVertexes[i]->setPos(xCoord+1200,yCoord+1100);
    }
    qDebug() << "build " << fff++;
    
    int count = 1;
    for (int i = 0 ; i < tSize ; i++)
    {
        for (int j = 0 ; j < tSize ; j++)
        {
            if (table->item(i,j)->text() =="1")
            {
                qDebug() << "build " << fff++;
                
                Edge::ArrowType arrType;
                bool isCopy = false;
                QString name = "e"+QString::number(count++);
                Vertex *startItem,*endItem;
                if (table->item(j,i)->text() == "1")
                {
                    qDebug() << "build " << fff++;
                    
                    if (i < j)
                        arrType  = Edge::EdgeType;
                    else if (i == j)
                        arrType = Edge::LoopType;
                    else
                        isCopy = true;
                }
                else
                    arrType = Edge::EdgeType;
                if (isCopy == false)
                {
                    qDebug() << "build " << fff++;
                    
                    foreach(QGraphicsItem* item,scene->items())
                    {
                        if (item->type() == Vertex::Type)
                        {
                            Vertex *curVertex = qgraphicsitem_cast<Vertex *>(item);
                            if(curVertex->getName() == tableLabels[i])
                                startItem = curVertex;
                            if(curVertex->getName() == tableLabels[j])
                                endItem = curVertex;
                        }
                    }
                    qDebug() << "build " << fff++;
                    
                    scene->addEdge(name,startItem,endItem,arrType);
                }
                else
                    isCopy = false;
            }
            qDebug() << "build " << fff++;
            
        }
        qDebug() << "build " << fff++;
        
    }
    qDebug() << "build " << fff++;
    
    this->statusBar()->showMessage("���� ��������");

    this->activeGraphicsView()->centerScrollBar();
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ������ ����� �� ������
void MainWindow::filePrint()
{
    this->statusBar()->showMessage("������ �����");

#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted) {
        QPainter paintPrint(&printer);
        paintPrint.drawPixmap(QPointF(0,0),getPixmap());
        paintPrint.end();
    }
    delete dlg;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������������� ��������� ��������
void MainWindow::filePrintPreview()
{
    this->statusBar()->showMessage("��������������� �������� �����");

#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    printer.setPaperSize(QPrinter::A4);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreview(QPrinter*)));
    preview.exec();
#endif
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� ���������������� ���������
void MainWindow::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    QPainter paintPrint(printer);
    paintPrint.drawPixmap(QPointF(0,0),getPixmap());
    paintPrint.end();
#endif
}
////////////////////////////////////////////////////////////////////////////////////////
// ������� �������� ����������� �� �����
QPixmap MainWindow::getPixmap()
{

    QPixmap pixmap(3000,3000);
    pixmap.fill();
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF rect=scene->itemsBoundingRect();
    rect.adjust(-10,-10,10,10);
    scene->render(&painter,QRectF(),rect);
    painter.end();
    
    return pixmap;
}
