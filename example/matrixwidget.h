#pragma once

#include <QWidget>
#include <QMatrix4x4>
#include <QDoubleSpinBox>

class MatrixWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MatrixWidget(QWidget *parent = nullptr);
    explicit MatrixWidget(const QMatrix4x4& matrix, QWidget *parent = nullptr);

    void setMatrix(const QMatrix4x4& matrix);
    QMatrix4x4 getMatrix() const;

signals:
    void matrixChanged();

private:
    void createWidgets();
    QDoubleSpinBox* createCellEditor(int row, int column);

    QMatrix4x4 matrix;
    QVector<QVector<QDoubleSpinBox*>> widgets;
};
