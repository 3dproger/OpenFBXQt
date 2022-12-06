#ifndef OPENFBXQT3D_H
#define OPENFBXQT3D_H

#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QJoint>
#include <Qt3DRender/QMaterial>
#include "ofbx.h"

class OpenFBXQt3D
{
public:
    struct JointInfo
    {
        uint index = 0;
        Qt3DCore::QJoint* joint = nullptr;
        const ofbx::Cluster* cluster = nullptr;
    };

    static Qt3DCore::QEntity* load(const QString& fileName, QHash<QString, JointInfo>& jointsOutput, Qt3DCore::QEntity* parent);

private:
    OpenFBXQt3D(){}
};

#endif // OPENFBXQT3D_H
