#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QLabel>

namespace
{

enum class ItemType { NotSetted, Model, Armature, Joint };
static const int ItemTypeRole = Qt::UserRole + 1;
static const int ItemPointerRole = Qt::UserRole + 2;

void clearLayout(QLayout& layout)
{
    QLayoutItem *item;
    while ((item = layout.takeAt(0)))
    {
        if (item->layout())
        {
            clearLayout(*item->layout());
            item->layout()->deleteLater();
        }

        if (item->widget())
        {
           item->widget()->deleteLater();
        }

        delete item;
    }
}

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , infoIcon(":/images/Info.svg")
    , warningIcon(":/images/Warning.svg")
    , errorIcon(":/images/Error.svg")
    , jointIcon(":/images/joint.png")
    , materialIcon(":/images/material.png")
    , modelIcon(":/images/model.png")
    , armatureIcon(":/images/armature.png")
    , textureIcon(":/images/texture.png")
{
    ui->setupUi(this);

    ui->bottomPanelSplitter->setCollapsible(0, false);
    ui->bottomPanelSplitter->setSizes({ 100, 0 });
    ui->rightPanelSplitter->setSizes({ 1000, 100 });
    ui->rightPanelSplitter->setCollapsible(0, false);

    updateSceneTree();
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
    updateSceneTree();
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::open(const QString &fileName)
{
    addLogMessage(ofbxqt::Note(ofbxqt::Note::Type::Info, tr("Opening file \"%1\"").arg(fileName)));

    ofbxqt::Scene& scene = ui->sceneWidget->scene;

    ofbxqt::OpenModelConfig config;

    QList<ofbxqt::Note> notes;
    const QVector<std::shared_ptr<ofbxqt::Model>> models = scene.open(fileName, config, &notes);

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

    updateSceneTree();
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

void MainWindow::updateSceneTree()
{
    QTreeWidget& tree = *ui->sceneTree;
    tree.clear();
    updateInspector();
    const ofbxqt::Scene& scene = ui->sceneWidget->scene;
    const QVector<std::shared_ptr<ofbxqt::Model>>& models = scene.getModels();
    if (models.isEmpty())
    {
        QTreeWidgetItem* item = new QTreeWidgetItem({ tr("Empty") });
        tree.addTopLevelItem(item);
        return;
    }

    for (int modelIndex = 0; modelIndex < models.count(); ++modelIndex)
    {
        const std::shared_ptr<ofbxqt::Model> model = models[modelIndex];
        QTreeWidgetItem* modelItem = new QTreeWidgetItem({ tr("Model %1").arg(modelIndex) });
        modelItem->setIcon(0, modelIcon);
        tree.addTopLevelItem(modelItem);
        modelItem->setExpanded(true);
        modelItem->setData(0, ItemTypeRole, (int)ItemType::Model);
        modelItem->setData(0, ItemPointerRole, (uint64_t)model.get());

        if (model->armature.getRootJoint())
        {
            QTreeWidgetItem* armatureItem = new QTreeWidgetItem({ tr("Armature")});
            armatureItem->setIcon(0, armatureIcon);
            armatureItem->setData(0, ItemTypeRole, (int)ItemType::Armature);
            armatureItem->setData(0, ItemPointerRole, (uint64_t)&model->armature);

            fillJointItem(*armatureItem, { model->armature.getRootJoint() });

            modelItem->addChild(armatureItem);
        }
    }

    updateInspector();
}

void MainWindow::fillJointItem(QTreeWidgetItem &parentItem, const QVector<std::shared_ptr<ofbxqt::Joint>>& joints)
{
    for (const std::shared_ptr<ofbxqt::Joint>& joint : joints)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem({ joint->getName() });
        item->setIcon(0, jointIcon);
        item->setData(0, ItemTypeRole, (int)ItemType::Joint);
        item->setData(0, ItemPointerRole, (uint64_t)joint.get());

        fillJointItem(*item, joint->getChildren());

        parentItem.addChild(item);
    }
}

void MainWindow::on_sceneTree_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
    updateInspector();
}

void MainWindow::updateInspector()
{
    QGridLayout& layout = *ui->inspectorLayout;
    clearLayout(layout);

    QTreeWidgetItem* item = ui->sceneTree->currentItem();
    if (!item)
    {
        return;
    }

    const ItemType itemType = (ItemType)item->data(0, ItemTypeRole).toInt();
    if (itemType == ItemType::NotSetted)
    {
        return;
    }

    void* object = (void*)item->data(0, ItemPointerRole).toULongLong();
    if (!object)
    {
        qCritical() << Q_FUNC_INFO << "object is null";
        return;
    }

    if (itemType == ItemType::Model)
    {
        ofbxqt::Model* model = (ofbxqt::Model*)object;
        layout.addWidget(new QLabel("Model", this));
    }
    else if (itemType == ItemType::Armature)
    {
        ofbxqt::Armature* armature = (ofbxqt::Armature*)object;
        layout.addWidget(new QLabel("Armature", this));
    }
    else if (itemType == ItemType::Joint)
    {
        ofbxqt::Joint* joint = (ofbxqt::Joint*)object;
        layout.addWidget(new QLabel(joint->getName(), this));
    }
    else
    {
        qCritical() << Q_FUNC_INFO << "unknown item type" << (int)itemType;
    }

    layout.addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), layout.rowCount(), 0);
}
