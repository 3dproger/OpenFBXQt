#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "transformwidget.h"
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QSlider>
#include <QApplication>
#include <QDoubleSpinBox>

namespace
{

enum class ItemType { NotSetted, File, Model, Armature, Joint };
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
    , fileIcon(":/images/file.png")
    , jointIcon(":/images/joint.png")
    , materialIcon(":/images/material.png")
    , modelIcon(":/images/model.png")
    , armatureIcon(":/images/armature.png")
    , textureIcon(":/images/texture.png")
{
    ui->setupUi(this);

    setWindowTitle(QCoreApplication::applicationName() + " (library version \"" + ofbxqt::VersionLib + "\")");

    ui->bottomPanelSplitter->setCollapsible(0, false);
    ui->bottomPanelSplitter->setSizes({ 100, 0 });
    ui->rightPanelSplitter->setSizes({ 1000, 230 });
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

    const ofbxqt::FileInfo fileInfo = scene.open(fileName, config);

    for (const ofbxqt::Note& note : qAsConst(fileInfo.notes))
    {
        addLogMessage(note);
    }

    if (fileInfo.topLevelModels.isEmpty())
    {
        addLogMessage(ofbxqt::Note(ofbxqt::Note::Type::Error, tr("No open models")));
    }
    else
    {
        addLogMessage(ofbxqt::Note(ofbxqt::Note::Type::Info, tr("Opened %1 top level model(s)").arg(fileInfo.topLevelModels.count())));
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
    const QVector<std::shared_ptr<ofbxqt::FileInfo>>& files = ui->sceneWidget->scene.getFiles();
    if (files.isEmpty())
    {
        QTreeWidgetItem* item = new QTreeWidgetItem({ tr("Empty") });
        tree.addTopLevelItem(item);
        return;
    }

    for (const std::shared_ptr<ofbxqt::FileInfo>& file : files)
    {
        QTreeWidgetItem* fileItem = new QTreeWidgetItem({ file->fileName });
        fileItem->setIcon(0, fileIcon);
        fileItem->setData(0, ItemTypeRole, (int)ItemType::File);
        fileItem->setData(0, ItemPointerRole, (uint64_t)file.get());
        tree.addTopLevelItem(fileItem);

        for (int i = 0; i < file->topLevelModels.count(); ++i)
        {
            fileItem->addChild(createModelItem(file->topLevelModels[i]));
        }
    }

    updateInspector();
}

QTreeWidgetItem *MainWindow::createModelItem(const std::shared_ptr<ofbxqt::Model> &model)
{
    QTreeWidgetItem* modelItem = new QTreeWidgetItem({ model->getName() });
    modelItem->setIcon(0, modelIcon);
    modelItem->setData(0, ItemTypeRole, (int)ItemType::Model);
    modelItem->setData(0, ItemPointerRole, (uint64_t)model.get());

    if (model->armature)
    {
        QTreeWidgetItem* armatureItem = new QTreeWidgetItem({ tr("Armature")});
        armatureItem->setIcon(0, armatureIcon);
        armatureItem->setData(0, ItemTypeRole, (int)ItemType::Armature);
        armatureItem->setData(0, ItemPointerRole, (uint64_t)model->armature.get());

        fillJointItem(*armatureItem, model->armature->getTopLevelJoints());

        modelItem->addChild(armatureItem);
    }

    for (const std::shared_ptr<ofbxqt::Model>& child : qAsConst(model->children))
    {
        modelItem->addChild(createModelItem(child));
    }

    return modelItem;
}

void MainWindow::fillJointItem(QTreeWidgetItem &parentItem, const QVector<std::shared_ptr<ofbxqt::Joint>>& joints)
{
    for (const std::shared_ptr<ofbxqt::Joint>& joint : joints)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem({ joint->getName() });
        item->setIcon(0, jointIcon);
        item->setData(0, ItemTypeRole, (int)ItemType::Joint);
        item->setData(0, ItemPointerRole, (uint64_t)joint.get());

        fillJointItem(*item, joint->children);

        parentItem.addChild(item);
    }
}

void MainWindow::on_sceneTree_currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
    updateInspector();
}

void MainWindow::updateInspector()
{
    QVBoxLayout& layout = *ui->propertiesLayout;
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

    if (itemType == ItemType::File)
    {
        ofbxqt::FileInfo* file = (ofbxqt::FileInfo*)object;

        QHBoxLayout* titleLayout = new QHBoxLayout(this);

        QLabel* labelIcon = new QLabel();
        labelIcon->setPixmap(QPixmap(":/images/file.png"));
        titleLayout->addWidget(labelIcon);

        QLabel* labelName = new QLabel(file->fileName, this);
        labelName->setWordWrap(true);
        titleLayout->addWidget(labelName);

        titleLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Policy::MinimumExpanding));
        layout.addLayout(titleLayout);
    }
    else if (itemType == ItemType::Model)
    {
        ofbxqt::Model* model = (ofbxqt::Model*)object;

        QHBoxLayout* titleLayout = new QHBoxLayout(this);

        QLabel* labelIcon = new QLabel();
        labelIcon->setPixmap(QPixmap(":/images/model.png"));
        titleLayout->addWidget(labelIcon);

        QLabel* labelName = new QLabel(model->getName(), this);
        labelName->setWordWrap(true);
        titleLayout->addWidget(labelName);

        titleLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Policy::MinimumExpanding));
        layout.addLayout(titleLayout);

        if (!model->children.isEmpty())
        {
            QLabel* label = new QLabel(tr("Children: %1").arg(model->children.count()), this);
            label->setWordWrap(true);
            layout.addWidget(label);
        }

        if (model->material)
        {
            QString text;

            if (model->material->diffuseTexture)
            {
                text += tr("Diffuse texture \"%1\"").arg(model->material->diffuseTexture->getFileName()) + "\n";
            }

            if (model->material->diffuseColor)
            {
                text += tr("Diffuse color \"%1\"").arg(model->material->diffuseColor->name()) + "\n";
            }

            if (model->material->normalTexture)
            {
                text += tr("Normal texture \"%1\"").arg(model->material->diffuseTexture->getFileName()) + "\n";
            }

            QLabel* textLabel = new QLabel(text, this);
            textLabel->setWordWrap(true);
            layout.addWidget(textLabel);
        }
        else
        {
            layout.addWidget(new QLabel(tr("No material"), this));
        }

        TransformWidget* transformWidget = new TransformWidget(model->getTransform());
        layout.addWidget(transformWidget);
        QObject::connect(transformWidget, &TransformWidget::transformChanged, this, [this, transformWidget, model]()
        {
            model->setTransform(transformWidget->getTransform());
            ui->sceneWidget->update();
        });
    }
    else if (itemType == ItemType::Armature)
    {
        ofbxqt::Armature* armature = (ofbxqt::Armature*)object;

        QHBoxLayout* titleLayout = new QHBoxLayout(this);
        QLabel* labelIcon = new QLabel();
        labelIcon->setPixmap(QPixmap(":/images/armature.png"));
        titleLayout->addWidget(labelIcon);
        titleLayout->addWidget(new QLabel("Armature", this));
        titleLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Policy::MinimumExpanding));
        layout.addLayout(titleLayout);

        layout.addWidget(new QLabel(tr("Joints count %1").arg(armature->getAllJoints().count()), this));
    }
    else if (itemType == ItemType::Joint)
    {
        ofbxqt::Joint* joint = (ofbxqt::Joint*)object;

        QHBoxLayout* titleLayout = new QHBoxLayout(this);
        QLabel* labelIcon = new QLabel();
        labelIcon->setPixmap(QPixmap(":/images/joint.png"));
        titleLayout->addWidget(labelIcon);
        titleLayout->addWidget(new QLabel(joint->getName(), this));
        titleLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Policy::MinimumExpanding));
        layout.addLayout(titleLayout);

        TransformWidget* transformWidget = new TransformWidget(joint->getTransform());
        layout.addWidget(transformWidget);
        QObject::connect(transformWidget, &TransformWidget::transformChanged, this, [this, transformWidget, joint]()
        {
            joint->setTransform(transformWidget->getTransform());
            ui->sceneWidget->update();
        });
    }
    else
    {
        qCritical() << Q_FUNC_INFO << "unknown item type" << (int)itemType;
    }

    layout.addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
}
