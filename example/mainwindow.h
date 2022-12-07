#pragma once

#include "openfbxqt.h"
#include "joint.h"
#include <QMainWindow>
#include <QTreeWidgetItem>

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
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionExit_triggered();

private:
    void open(const QString& fileName);
    void addLogMessage(const ofbxqt::Note& note);
    void updateSceneTree();
    void fillJointItem(QTreeWidgetItem& parentItem, const QVector<std::shared_ptr<ofbxqt::Joint>>& joints);

    Ui::MainWindow *ui;

    const QIcon infoIcon;
    const QIcon warningIcon;
    const QIcon errorIcon;

    const QIcon jointIcon;
    const QIcon materialIcon;
    const QIcon modelIcon;
    const QIcon skeletonIcon;
    const QIcon textureIcon;
};
