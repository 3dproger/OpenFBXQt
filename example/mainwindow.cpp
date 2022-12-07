#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->bottomPanelSplitter->setCollapsible(0, false);
    ui->bottomPanelSplitter->setSizes({ 100, 0 });

    ui->leftPanelSplitter->setSizes({ 1000, 100 });

    open("D:/Projects/openfbxqt/build-OpenFBXQtViewer-Desktop_Qt_5_15_2_MinGW_32_bit-Debug/glove1.fbx");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    const QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(), "*.fbx").trimmed();
    if (fileName.isEmpty())
    {
        return;
    }

    open(fileName);
}

void MainWindow::on_actionClose_triggered()
{
    ui->logWidget->clear();
    ofbxqt::Scene& scene = ui->sceneWidget->scene;
    scene.clear();
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::open(const QString &fileName)
{
    addLogMessage(ofbxqt::Note(ofbxqt::Note::Type::Info, tr("Opening file \"%1\"").arg(fileName)));

    ofbxqt::Scene& scene = ui->sceneWidget->scene;

    QList<ofbxqt::Note> notes;
    const QList<ofbxqt::Model*> models = scene.open(fileName, notes);
    for (const ofbxqt::Note& note : qAsConst(notes))
    {
        addLogMessage(note);
    }

    if (models.isEmpty())
    {
        addLogMessage(ofbxqt::Note(ofbxqt::Note::Type::Error, tr("Failed to open file \"%1\"").arg(fileName)));
    }
    else
    {
        addLogMessage(ofbxqt::Note(ofbxqt::Note::Type::Info, tr("Open %1 model(s)").arg(models.count())));
    }

    // ============= TEST ==================
    if (!models.isEmpty())
    {
        if (ofbxqt::Joint* j = models[0]->skeleton.getJointByName("Bone.Forefinger.002"); j)
        {
            j->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1).normalized(), -30));
        }

        if (ofbxqt::Joint* j = models[0]->skeleton.getJointByName("Bone.Forefinger.003"); j)
        {
            j->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1).normalized(), -45));
        }

        models[0]->skeleton.update();
    }
    // ============= TEST ==================
}

void MainWindow::addLogMessage(const ofbxqt::Note &note)
{
    ui->logWidget->addItem(note.getText());

    if (note.getType() != ofbxqt::Note::Type::Info)
    {
        ui->bottomPanelSplitter->setSizes({1000, 200});
    }
}

