#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , infoIcon(":/images/Info.svg")
    , warningIcon(":/images/Warning.svg")
    , errorIcon(":/images/Error.svg")
{
    ui->setupUi(this);

    ui->bottomPanelSplitter->setCollapsible(0, false);
    ui->bottomPanelSplitter->setSizes({ 100, 0 });
    ui->leftPanelSplitter->setSizes({ 1000, 100 });
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
    ui->sceneWidget->resetCamera();
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
    ofbxqt::OpenModelConfig config;

    const QList<ofbxqt::Model*> models = scene.open(fileName, config, &notes);
    for (const ofbxqt::Note& note : qAsConst(notes))
    {
        addLogMessage(note);
    }

    if (models.isEmpty())
    {
        addLogMessage(ofbxqt::Note(ofbxqt::Note::Type::Error, tr("No open models")));
    }
    else
    {
        addLogMessage(ofbxqt::Note(ofbxqt::Note::Type::Info, tr("Open %1 model(s)").arg(models.count())));
    }
}

void MainWindow::addLogMessage(const ofbxqt::Note &note)
{
    QListWidgetItem* item = new QListWidgetItem(note.getText());

    switch (note.getType())
    {
    case ofbxqt::Note::Type::Info:
        item->setIcon(infoIcon);
        break;
    case ofbxqt::Note::Type::Warning:
        item->setIcon(warningIcon);
        break;
    case ofbxqt::Note::Type::Error:
        item->setIcon(errorIcon);
        break;
    }

    ui->logWidget->addItem(item);

    if (note.getType() != ofbxqt::Note::Type::Info)
    {
        ui->bottomPanelSplitter->setSizes({1000, 200});
    }

    ui->logWidget->scrollToBottom();
}

