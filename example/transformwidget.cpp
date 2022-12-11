#include "transformwidget.h"
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QMatrix4x4>
#include <QSlider>

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
    setLayout(layout);

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox);
        spinBox->setMinimum(-1000000);
        spinBox->setMaximum(1000000);
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
        layout->addWidget(spinBox);
        spinBox->setMinimum(-1000000);
        spinBox->setMaximum(1000000);
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
        layout->addWidget(spinBox);
        spinBox->setMinimum(-1000000);
        spinBox->setMaximum(1000000);
        spinBox->setValue(transform.getTanslation().y());

        QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
        {
            QVector3D translation = transform.getTanslation();
            translation.setZ(value);
            transform.setTranslation(translation);
            emit transformChanged();
        });
    }

    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        layout->addWidget(spinBox);
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(1000000);
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
        layout->addWidget(spinBox);
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(1000000);
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
        layout->addWidget(spinBox);
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(1000000);
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

    {
        QSlider* slider = new QSlider(Qt::Orientation::Horizontal, this);
        layout->addWidget(slider);
        slider->setMinimum(-180);
        slider->setMaximum(180);
        slider->setValue(transform.getEulerAngles().x());
        QObject::connect(slider, &QSlider::valueChanged, this, [this](int value)
        {
            QVector3D eulerAngles = transform.getEulerAngles();
            eulerAngles.setX(value);
            transform.setEulerAngles(eulerAngles);
            emit transformChanged();
        });
    }

    {
        QSlider* slider = new QSlider(Qt::Orientation::Horizontal, this);
        layout->addWidget(slider);
        slider->setMinimum(-180);
        slider->setMaximum(180);
        slider->setValue(transform.getEulerAngles().y());
        QObject::connect(slider, &QSlider::valueChanged, this, [this](int value)
        {
            QVector3D eulerAngles = transform.getEulerAngles();
            eulerAngles.setY(value);
            transform.setEulerAngles(eulerAngles);
            emit transformChanged();
        });
    }

    {
        QSlider* slider = new QSlider(Qt::Orientation::Horizontal, this);
        layout->addWidget(slider);
        slider->setMinimum(-180);
        slider->setMaximum(180);
        slider->setValue(transform.getEulerAngles().z());
        QObject::connect(slider, &QSlider::valueChanged, this, [this](int value)
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
