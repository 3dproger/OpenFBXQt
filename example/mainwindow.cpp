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

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::open(const QString &fileName)
{
    QList<ofbxqt::Note> notes;
    const QList<ofbxqt::Model*> models = ui->sceneWidget->scene.open(fileName, notes);

    QString errorText;

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
        errorText = errorText.trimmed();
        if (errorText.isEmpty())
        {
            errorText = tr("Unknown error");
        }

        QMessageBox::critical(this, QString(), errorText);
    }
    else
    {
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
    }
}

