#pragma once

#include "openfbxqt.h"
#include "scene.h"
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

    void on_sceneTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
    void open(const QString& fileName);
    void addLogMessage(const ofbxqt::Note& note);
    void updateSceneTree();
    QTreeWidgetItem* createModelItem(const std::shared_ptr<ofbxqt::Model>& model);
    void fillJointItem(QTreeWidgetItem& parentItem, const QVector<std::shared_ptr<ofbxqt::Joint>>& joints);
    void updateInspector();

    Ui::MainWindow *ui;

    const QIcon infoIcon;
    const QIcon warningIcon;
    const QIcon errorIcon;

    const QIcon fileIcon;
    const QIcon jointIcon;
    const QIcon materialIcon;
    const QIcon modelIcon;
    const QIcon armatureIcon;
    const QIcon textureIcon;
};
