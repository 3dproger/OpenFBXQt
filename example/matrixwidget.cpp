#include "matrixwidget.h"
#include <QGridLayout>
#include <limits>
#include <QPushButton>

MatrixWidget::MatrixWidget(QWidget *parent)
    : QWidget{parent}
{
    createWidgets();
}

MatrixWidget::MatrixWidget(const QMatrix4x4 &matrix_, QWidget *parent)
    : QWidget{parent}
    , matrix(matrix_)
{
    createWidgets();
}

void MatrixWidget::setMatrix(const QMatrix4x4 &matrix_)
{
    matrix = matrix_;

    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 4; ++column)
        {
            const double value = matrix.data()[row + column * 4];
            widgets.at(row).at(column)->setValue(value);
        }
    }
}

QMatrix4x4 MatrixWidget::getMatrix() const
{
    return matrix;
}

void MatrixWidget::createWidgets()
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    setLayout(layout);

    for (int row = 0; row < 4; ++row)
    {
        QVector<QDoubleSpinBox*> rowWidgets;

        for (int column = 0; column < 4; ++column)
        {
            QDoubleSpinBox* widget = createCellEditor(row, column);
            layout->addWidget(widget, row, column);
            rowWidgets.append(widget);
        }

        widgets.append(rowWidgets);
    }

    {
        QHBoxLayout* buttonLayout = new QHBoxLayout(this);
        layout->addLayout(buttonLayout, 4, 0, 1, 4);
        layout->setMargin(0);

        QPushButton* button = new QPushButton(tr("Reset"), this);
        buttonLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Maximum));
        buttonLayout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, this, [this]()
        {
            setMatrix(QMatrix4x4());
            emit matrixChanged();
        });
    }
}

QDoubleSpinBox* MatrixWidget::createCellEditor(const int row, const int column)
{
    if (column > 4 || column < 0 || row > 4 || row < 0)
    {
        qCritical() << Q_FUNC_INFO << "invalid column or row";
        return nullptr;
    }

    QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
    spinBox->setMinimum(std::numeric_limits<double>::lowest());
    spinBox->setMaximum(std::numeric_limits<double>::max());
    spinBox->setValue(matrix.constData()[row + column * 4]);
    spinBox->setDecimals(5);

    QObject::connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, row, column](double value)
    {
        matrix.data()[row + column * 4] = value;
        emit matrixChanged();
    });

    return spinBox;
}
