#include "ui.h"

Ui::Ui()
{
    this->resize(1024, 768);
    createActions();
    createQVTKWidget();
    setupMenuBar();
    setupStatusBar();
    setupBoxAndLayout();
    setWindowTitle(tr("PCQC - Point Cloud Quality Control"));
}

void Ui::about()
{
   QMessageBox::about(this, tr("About pcl-exam-project"), tr("The <b>pcl-exam-project</b> is super awesome") );
}

void Ui::aboutPCL()
{
   QMessageBox::about(this, tr("About PCL"), tr("The Point Cloud Library (or PCL) is a large scale,"
                                                "open project for 3D point cloud processing.") );
}

void Ui::createActions()
{
    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    aboutPCLAct = new QAction(tr("About &PCL"), this);
    aboutPCLAct->setStatusTip(tr("Show the PCL library's About box"));
    connect(aboutPCLAct, SIGNAL(triggered()), this, SLOT(aboutPCL()));
}

void Ui::createQVTKWidget()
{
    // setup widget
    qvtkVisualizer = new QVTKWidget();
    viewer = new pcl::visualization::PCLVisualizer("3DViewer", false);// don't display in the vtk visualizer, render it on a qt window
    qvtkVisualizer->SetRenderWindow(viewer->getRenderWindow());
    // setup pcl visualizer
    viewer->setupInteractor(qvtkVisualizer->GetInteractor(), qvtkVisualizer->GetRenderWindow());// tells the viewer what interactor and what window is using now
    viewer->getInteractorStyle()->setKeyboardModifier(pcl::visualization::INTERACTOR_KB_MOD_SHIFT);// ripristina input system of original visualizer (shift+click for points)
    viewer->setBackgroundColor(0, 0, 0);
    viewer->initCameraParameters();
    viewer->registerPointPickingCallback(&pointPickCallback, this);
//    qvtkVisualizer->resize(640, 480);
//    setCentralWidget(qvtkVisualizer);
}

void Ui::setupMenuBar()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(quitAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
    helpMenu->addAction(aboutPCLAct);
}

void Ui::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void Ui::setupBoxAndLayout
()
{
    loadTBox = new QGroupBox(QString("Load Target Cloud"));
    browseButton = new QPushButton(QString("Browse..."));
    pathField = new QLineEdit();
    loadButton = new QPushButton(QString("LOAD!"));
    loadTargetLayout = new QHBoxLayout;
    loadTargetLayout->addWidget(browseButton);
    loadTargetLayout->addWidget(pathField);
    loadTargetLayout->addWidget(loadButton);
    loadTBox->setLayout(loadTargetLayout);

    componentBox = new QGroupBox(QString("Components definition"));
    componentButtonsLayout = new QHBoxLayout;
    addComponentButton = new QPushButton(QString("Add..."));
    delComponentButton = new QPushButton(QString("Delete"));
    componentButtonsLayout->addWidget(addComponentButton);
    componentButtonsLayout->addWidget(delComponentButton);
    componentLayout = new QVBoxLayout;
    componentLayout->addLayout(componentButtonsLayout);
    componentList = new QListWidget;
    componentLayout->addWidget(componentList);

    checkLayout = new QVBoxLayout;
    //hlayout+list
    loadSourceLayout = new QHBoxLayout;
    //buttons+text
    resultsLayout = new QHBoxLayout;
    //button+list


    showTargetComponent = new QHBoxLayout;
    //button+menu a tendina
    showSourceComponent = new QHBoxLayout;
    //button+menu a tendina

    commandLayout = new QVBoxLayout;
    viewerLayout = new QVBoxLayout;


    mainLayout = new QHBoxLayout;
    mainLayout->addLayout(commandLayout);
    mainLayout->addLayout(viewerLayout);
}

void Ui::pointPickCallback(const pcl::visualization::PointPickingEvent& event, void* cookie)
{
    Ui *ui = (Ui*)cookie;
    float x,y,z;
    if (event.getPointIndex() == -1)
        ui->statusBar()->showMessage(tr("No point was clicked"));
    else
    {
        event.getPoint(x,y,z);
        ui->statusBar()->showMessage(QString("Point Clicked index: %1 x: %2 y: %3 z: %4")
                                 .arg(event.getPointIndex())
                                 .arg(x)
                                 .arg(y)
                                 .arg(z)
                                 );
    }
}
