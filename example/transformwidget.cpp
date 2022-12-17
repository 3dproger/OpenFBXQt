#include "transformwidget.h"
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QLabel>

TransformWidget::TransformWidget(QWidget *parent)
    : QWidget{parent}
{
    createWidgets();
}

TransformWidget::TransformWidget(const ofbxqt::Transform &transform_, QWidget *parent)
    : QWidget{parent}
    , transform(transform_)
{
    createWidgets();
}

void TransformWidget::createWidgets()
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    setLayout(layout);

    int row = 0;
    QLabel* label;

    label = new QLabel("X", this);
    label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    layout->addWidget(label, row, 1);

    label = new QLabel("Y", this);
    label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    layout->addWidget(label, row, 2);

    label = new QLabel("Z", this);
    label->setAlignment(Qt::AlignmentFlag::AlignCenter);
    layout->addWidget(label, row, 3);

    row++;

    label = new QLabel(this);
    label->setPixmap(QPixmap(":/images/translation.png"));
    layout->addWidget(label, row, 0);

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 1);
        spinBox->setSingleStep(10);
        spinBox->setMinimum(-100000);
        spinBox->setMaximum(100000);
        spinBox->setValue(transform.getTanslation().x());

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D translation = transform.getTanslation();
            translation.setX(value);
            transform.setTranslation(translation);
            emit transformChanged();
        });
    }

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 2);
        spinBox->setSingleStep(10);
        spinBox->setMinimum(-100000);
        spinBox->setMaximum(100000);
        spinBox->setValue(transform.getTanslation().y());

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D translation = transform.getTanslation();
            translation.setY(value);
            transform.setTranslation(translation);
            emit transformChanged();
        });
    }

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 3);
        spinBox->setSingleStep(10);
        spinBox->setMinimum(-100000);
        spinBox->setMaximum(100000);
        spinBox->setValue(transform.getTanslation().z());

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D translation = transform.getTanslation();
            translation.setZ(value);
            transform.setTranslation(translation);
            emit transformChanged();
        });
    }

    row++;

    label = new QLabel(this);
    label->setPixmap(QPixmap(":/images/scale.png"));
    layout->addWidget(label, row, 0);

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 1);
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(100000);
        spinBox->setValue(transform.getScale().x());
        spinBox->setStepType(QDoubleSpinBox::StepType::AdaptiveDecimalStepType);

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D scale = transform.getScale();
            scale.setX(value);
            transform.setScale(scale);
            emit transformChanged();
        });
    }

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 2);
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(100000);
        spinBox->setValue(transform.getScale().y());
        spinBox->setStepType(QDoubleSpinBox::StepType::AdaptiveDecimalStepType);

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D scale = transform.getScale();
            scale.setY(value);
            transform.setScale(scale);
            emit transformChanged();
        });
    }

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 3);
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(100000);
        spinBox->setValue(transform.getScale().z());
        spinBox->setStepType(QDoubleSpinBox::StepType::AdaptiveDecimalStepType);

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D scale = transform.getScale();
            scale.setZ(value);
            transform.setScale(scale);
            emit transformChanged();
        });
    }

    row++;

    label = new QLabel(this);
    label->setPixmap(QPixmap(":/images/rotation.png"));
    layout->addWidget(label, row, 0);

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 1);
        spinBox->setSingleStep(10);
        spinBox->setMinimum(-180);
        spinBox->setMaximum(180);
        spinBox->setValue(transform.getEulerAngles().x());

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D eulerAngles = transform.getEulerAngles();
            eulerAngles.setX(value);
            transform.setEulerAngles(eulerAngles);
            emit transformChanged();
        });
    }

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 2);
        spinBox->setSingleStep(10);
        spinBox->setMinimum(-180);
        spinBox->setMaximum(180);
        spinBox->setValue(transform.getEulerAngles().y());

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D eulerAngles = transform.getEulerAngles();
            eulerAngles.setY(value);
            transform.setEulerAngles(eulerAngles);
            emit transformChanged();
        });
    }

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox, row, 3);
        spinBox->setSingleStep(10);
        spinBox->setMinimum(-180);
        spinBox->setMaximum(180);
        spinBox->setValue(transform.getEulerAngles().z());

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D eulerAngles = transform.getEulerAngles();
            eulerAngles.setZ(value);
            transform.setEulerAngles(eulerAngles);
            emit transformChanged();
        });
    }
}

void TransformWidget::setTransform(const ofbxqt::Transform &transform_)
{
    transform = transform_;
}

ofbxqt::Transform TransformWidget::getTransform() const
{
    return transform;
}
