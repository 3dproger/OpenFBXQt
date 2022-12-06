#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loader.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

    QList<ofbxqt::Note> notes;
    const QList<ofbxqt::Model*> models = ofbxqt::Loader::load(fileName, notes);

    QString errorText = tr("File not loaded");

    bool foundError = false;
    for (const ofbxqt::Note& note : qAsConst(notes))
    {
        switch (note.getType())
        {
        case ofbxqt::Note::Type::Info:
            qInfo() << Q_FUNC_INFO << note.getText();
            break;
        case ofbxqt::Note::Type::Warning:
            qWarning() << Q_FUNC_INFO << note.getText();
            break;
        case ofbxqt::Note::Type::Error:
        {
            foundError = true;

            if (!errorText.isEmpty())
            {
                errorText += ". ";
            }

            errorText += note.getText();
        }
            break;
        }
    }

    if (foundError || models.isEmpty())
    {
        QMessageBox::critical(this, QString(), errorText);
        return;
    }

    // ============= TEST ==================
    if (ofbxqt::Joint* j = models[0]->skeleton.getJointByName("Bone.Forefinger.002"); j)
    {
        j->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1).normalized(), -30));
    }

    if (ofbxqt::Joint* j = models[0]->skeleton.getJointByName("Bone.Forefinger.003"); j)
    {
        j->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1).normalized(), -45));
    }

    models[0]->skeleton.update();
    // ============= TEST ==================

    qDebug() << "Loaded" << models.count() << "models";

    for (ofbxqt::Model* model : models)
    {
        ui->sceneWidget->scene.addModel(model);
    }
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

