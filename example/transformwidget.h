#pragma once

#include "openfbxqt.h"
#include <QWidget>

class TransformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TransformWidget(QWidget *parent = nullptr);
    explicit TransformWidget(const ofbxqt::Transform& transform, QWidget *parent = nullptr);

    void setTransform(const ofbxqt::Transform& transform);
    ofbxqt::Transform getTransform() const;

signals:
    void transformChanged();

private:
    void createWidgets();

    ofbxqt::Transform transform;
};
