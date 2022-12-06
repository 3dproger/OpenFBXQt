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
    ofbxqt::Model* model = ofbxqt::Loader::load(fileName, notes);

    QString errorText = tr("Model not loaded");

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
            if (!errorText.isEmpty())
            {
                errorText += ". ";
            }

            errorText += note.getText();
        }
            break;
        }
    }

    if (!model)
    {
        QMessageBox::critical(this, QString(), errorText);
        return;
    }

    // ============= TEST ==================
    if (ofbxqt::Joint* j = model->skeleton.getJointByName("Bone.Forefinger.002"); j)
    {
        j->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1).normalized(), -30));
    }

    if (ofbxqt::Joint* j = model->skeleton.getJointByName("Bone.Forefinger.003"); j)
    {
        j->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1).normalized(), -45));
    }
    // ============= TEST ==================

    model->skeleton.update();

    ui->sceneWidget->scene.addModel(model);
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

