#include "openfbxqt3d.h"
#include <QFile>
#include <QDebug>
#include <Qt3DRender>
#include <Qt3DExtras>

namespace
{

ofbx::IElement* childById(const ofbx::IElement* parent, const QString& id, const Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitivity::CaseSensitive)
{
    if (!parent)
    {
        return nullptr;
    }

    ofbx::IElement* child = parent->getFirstChild();
    while (child)
    {
        char rawId[256];
        child->getID().toString(rawId);
        if (QString::compare(QString::fromLatin1(rawId), id, caseSensitivity) == 0)
        {
            return child;
        }


        child = child->getSibling();
    }

    return nullptr;
}

QMatrix4x4 convertMatrix4x4(const ofbx::Matrix& source)
{
    return QMatrix4x4(source.m[0],  source.m[1],  source.m[2],  source.m[3],
                      source.m[4],  source.m[5],  source.m[6],  source.m[7],
                      source.m[8],  source.m[9],  source.m[10], source.m[11],
                      source.m[12], source.m[13], source.m[14], source.m[15]);
}

uint jointIndex = 0;

Qt3DCore::QJoint* makeJoint(const ofbx::Cluster* cluster, QHash<QString, OpenFBXQt3D::JointInfo>& jointsInfoOutput, Qt3DCore::QJoint* parent)
{
    if (!parent)
    {
        qWarning() << Q_FUNC_INFO << "parent is nullptr";
        return nullptr;
    }

    const ofbx::Object* object = cluster->getLink();
    if (!object)
    {
        qWarning() << Q_FUNC_INFO << "object of cluster is nullptr";
        return nullptr;
    }

    Qt3DCore::QJoint* joint = new Qt3DCore::QJoint(parent);
    joint->setName(object->name);

    joint->setInverseBindMatrix(convertMatrix4x4(cluster->getTransformLinkMatrix()));

    OpenFBXQt3D::JointInfo jointInfo;
    jointInfo.index = jointIndex++;
    jointInfo.joint = joint;

    jointsInfoOutput.insert(joint->name(), jointInfo);

    /*ofbx::IElement* childElement = object->element.getFirstChild();
    while (childElement)
    {


        childElement = childElement->getSibling();
    }*/

    return joint;
}

}

Qt3DCore::QEntity* OpenFBXQt3D::load(const QString& fileName, QHash<QString, JointInfo>& jointsOutput, Qt3DCore::QEntity* parent)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug(QString("Failed to open file \"%1\", error: \"%2\"").arg(fileName).arg(file.errorString()).toUtf8());
        return nullptr;
    }

    const QByteArray data = file.readAll();
    file.close();

    ofbx::IScene* scene = ofbx::load((ofbx::u8*)data.constData(), data.size(), (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
    if (!scene)
    {
        qCritical() << Q_FUNC_INFO << "no scene";
        return nullptr;
    }

    const int meshCount = scene->getMeshCount();
    if (meshCount <= 0)
    {
        qCritical() << Q_FUNC_INFO << "no meshes in scene";
        return nullptr;
    }

    const ofbx::Mesh* mesh = scene->getMesh(0);
    if (!mesh)
    {
        qCritical() << Q_FUNC_INFO << "mesh is null";
        return nullptr;
    }

    const ofbx::Geometry* rawGeometry = mesh->getGeometry();
    if (!rawGeometry)
    {
        qCritical() << Q_FUNC_INFO << "raw geometry is null";
        return nullptr;
    }

    const ofbx::Vec3* rawPositions = rawGeometry->getVertices();
    if (!rawPositions)
    {
        qCritical() << Q_FUNC_INFO << "raw positions is null";
        return nullptr;
    }

    const int* indices = rawGeometry->getFaceIndices();
    if (!indices)
    {
        qCritical() << Q_FUNC_INFO << "raw indices is null";
        return nullptr;
    }

    const ofbx::Vec3* rawNormals = rawGeometry->getNormals();
    if (!rawNormals)
    {
        qCritical() << Q_FUNC_INFO << "raw normals is null";
        return nullptr;
    }

    const ofbx::Vec2* rawTexcoord = rawGeometry->getUVs();
    if (!rawTexcoord)
    {
        qCritical() << Q_FUNC_INFO << "texture coordinats is null";
        return nullptr;
    }

    const int vertexCount  = rawGeometry->getVertexCount();
    const int indicesCount = rawGeometry->getIndexCount();

    //qDebug() << "vertexCount  =" << vertexCount;
    //qDebug() << "indicesCount =" << indicesCount;

    Qt3DCore::QEntity* entity = new Qt3DCore::QEntity(parent);

    Qt3DRender::QGeometryRenderer* renderer = new Qt3DRender::QGeometryRenderer(entity);
    Qt3DRender::QGeometry* geometry = new Qt3DRender::QGeometry(renderer);

    {
        const ofbx::Skin* skin = rawGeometry->getSkin();
        if (!skin)
        {
            qCritical() << Q_FUNC_INFO << "skin is null";
            return nullptr;
        }

        const int clusterCount = skin->getClusterCount();

        if (clusterCount <= 0)
        {
            qCritical() << Q_FUNC_INFO << "no clusters";
            return nullptr;
        }

        int ins = 0;
        int wei = 0;
        for (int clusterNum = 0; clusterNum < skin->getClusterCount(); ++clusterNum)
        {
            const ofbx::Cluster* cluster = skin->getCluster(clusterNum);
            ins += cluster->getIndicesCount();
            wei += cluster->getWeightsCount();

            //qDebug() << skin->getCluster(clusterNum)->getLink()->name;

            //qDebug() << "joint indices" << skin->getCluster(clusterNum)->getIndicesCount();
            //qDebug() << "joint wieghts" << skin->getCluster(clusterNum)->getWeightsCount();
        }

        //qDebug() << "total joints ins =" << ins;
        //qDebug() << "total joints wei =" << wei;

        /*for (int i = 0; i < skin->getCluster(0)->getIndicesCount(); ++i)
        {
            qDebug() << skin->getCluster(0)->getIndices()[i];
        }*/

        Qt3DCore::QArmature* armature = new Qt3DCore::QArmature(entity);
        Qt3DCore::QSkeleton* skeleton = new Qt3DCore::QSkeleton(armature);

        Qt3DCore::QJoint* rootJoint = new Qt3DCore::QJoint(skeleton);
        rootJoint->setName("root");

        jointIndex = 0;

        JointInfo rootJointInfo;
        rootJointInfo.index = jointIndex++;
        rootJointInfo.joint = rootJoint;

        jointsOutput.insert(rootJoint->name(), rootJointInfo);

        //int maxInd = 0;
        //int someInd = 0;

        QHash<const ofbx::Object*, JointInfo> objectsJointInfo;

        for (int clusterNum = 0; clusterNum < skin->getClusterCount(); ++clusterNum)
        {
            const ofbx::Cluster* cluster = skin->getCluster(clusterNum);
            if (!cluster)
            {
                qWarning() << Q_FUNC_INFO << "cluster is nullptr";
                continue;
            }

            const ofbx::Object* object = cluster->getLink();
            if (!object)
            {
                qWarning() << Q_FUNC_INFO << "object of cluster is nullptr";
                continue;
            }

            Qt3DCore::QJoint* joint = new Qt3DCore::QJoint(skeleton);
            joint->setName(object->name);
            joint->setInverseBindMatrix(convertMatrix4x4(cluster->getTransformLinkMatrix()));

            JointInfo jointInfo;
            jointInfo.cluster = cluster;
            jointInfo.index = jointIndex++;
            jointInfo.joint = joint;
            objectsJointInfo.insert(object, jointInfo);
            jointsOutput.insert(joint->name(), jointInfo);
        }

        for (const ofbx::Object* object : objectsJointInfo.keys())
        {
            const JointInfo& jointInfo = objectsJointInfo[object];
            if (!jointInfo.joint)
            {
                qWarning() << Q_FUNC_INFO << "jointInfo has nullptr joint";
            }

            bool addedToParent = false;

            const ofbx::Object* parent = object->getParent();
            if (parent /*&& parentObj->getType() == ofbx::Object::Type::CLUSTER*/)
            {
                if (objectsJointInfo.contains(parent))
                {
                    JointInfo& parentJointInfo = objectsJointInfo[parent];
                    if (parentJointInfo.joint)
                    {
                        //qDebug() << "added" << jointInfo.joint->name() << "to" << parentJointInfo.joint->name();
                        parentJointInfo.joint->addChildJoint(jointInfo.joint);
                        addedToParent = true;
                    }
                    else
                    {
                        qWarning() << Q_FUNC_INFO << "parent joint is nullptr";
                    }
                }
            }

            if (!addedToParent)
            {
                //qDebug() << "added" << jointInfo.joint->name() << "to" << rootJoint->name();
                rootJoint->addChildJoint(jointInfo.joint);
            }
        }

        /*for (int clusterNum = 0; clusterNum < skin->getClusterCount(); ++clusterNum)
        {
            const ofbx::Cluster* cluster = skin->getCluster(clusterNum);
            if (!cluster)
            {
                qWarning() << Q_FUNC_INFO << "cluster is nullptr";
                continue;
            }

            const ofbx::Object* object = cluster->getLink();
            if (!object)
            {
                qWarning() << Q_FUNC_INFO << "object of cluster is nullptr";
                continue;
            }

            const ofbx::Object* parentObj = object->getParent();
            if (parentObj && parentObj->getType() == ofbx::Object::Type::LIMB_NODE)
            {
                // this is not sub-root joint
                continue;
            }*/

            /*Qt3DCore::QJoint* joint = makeJoint(cluster, jointsOutput, rootJoint);
            if (joint)
            {
                rootJoint->addChildJoint(joint);
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "joint is nullptr";
                continue;
            }*/

            /*const int indicesCount = cluster->getIndicesCount();
            QByteArray indexJointArray;
            indexJointArray.resize(indicesCount * sizeof(int));
            int* rawIndexJointArray = reinterpret_cast<int*>(indexJointArray.data());
            int idx = 0;
            for (int i = 0; i < indicesCount; ++i)
            {
                rawIndexJointArray[idx++] = cluster->getIndices()[i];
                if (rawIndexJointArray[idx - 1] > maxInd)
                {
                    maxInd = rawIndexJointArray[idx - 1];
                }

                if (rawIndexJointArray[idx - 1] == 1000)
                {
                    someInd++;
                }
            }

            Qt3DRender::QBuffer* indexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
            indexBuffer->setData(indexJointArray);

            Qt3DRender::QAttribute* indexAttribute = new Qt3DRender::QAttribute(geometry);
            indexAttribute->setName(Qt3DRender::QAttribute::defaultJointIndicesAttributeName());
            indexAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
            indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedInt);
            indexAttribute->setBuffer(indexBuffer);
            indexAttribute->setCount(indicesCount);
            indexAttribute->setByteStride(sizeof(uint));

            geometry->addAttribute(indexAttribute);

            const int weightsCount = cluster->getWeightsCount();
            QByteArray weightsJointArray;
            weightsJointArray.resize(weightsCount * sizeof(float));
            float* rawWeightsJointArray = reinterpret_cast<float*>(weightsJointArray.data());
            idx = 0;
            for (int i = 0; i < weightsCount; ++i)
            {
                rawWeightsJointArray[idx++] = (float)cluster->getWeights()[i];
            }

            Qt3DRender::QBuffer* weightBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
            weightBuffer->setData(weightsJointArray);

            Qt3DRender::QAttribute* weightAttribute = new Qt3DRender::QAttribute(geometry);
            weightAttribute->setName(Qt3DRender::QAttribute::defaultJointWeightsAttributeName());
            weightAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
            weightAttribute->setDataType(Qt3DRender::QAttribute::Float);
            weightAttribute->setBuffer(weightBuffer);
            weightAttribute->setCount(weightsCount);
            weightAttribute->setByteStride(sizeof(float));

            geometry->addAttribute(weightAttribute);*/
            //break;
        //}


        //qDebug() << "maxInd" << maxInd;
        //qDebug() << "some ind" << someInd;

        //qDebug() << rootJoint->childJoints().count();



        skeleton->setRootJoint(rootJoint);
        armature->setSkeleton(skeleton);
        entity->addComponent(armature);
    }

    Qt3DRender::QBuffer* vertexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    Qt3DRender::QBuffer* indexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer, geometry);

    static const quint32 stride = (3 + 3 + 2 + 4) * sizeof(float) + (4) * sizeof(uint);

    QByteArray vertexArray;
    vertexArray.resize(vertexCount * stride);
    float* rawVertexArray = reinterpret_cast<float*>(vertexArray.data());
    int idx = 0;

    for (int i = 0; i < vertexCount; ++i)
    {
        rawVertexArray[idx++] = (float)rawPositions[i].x;
        rawVertexArray[idx++] = (float)rawPositions[i].y;
        rawVertexArray[idx++] = (float)rawPositions[i].z;
        rawVertexArray[idx++] = (float)rawNormals[i].x;
        rawVertexArray[idx++] = (float)rawNormals[i].y;
        rawVertexArray[idx++] = (float)rawNormals[i].z;
        rawVertexArray[idx++] = (float)rawTexcoord[i].x;
        rawVertexArray[idx++] = (float)rawTexcoord[i].y;

        rawVertexArray[idx++] = (float)((i % 4 == 0) ? 0 : 0.5);
        rawVertexArray[idx++] = (float)0.5;
        rawVertexArray[idx++] = (float)0.2;
        rawVertexArray[idx++] = (float)0.1;

        rawVertexArray[idx++] = (float)-10.0;//dumb value
        rawVertexArray[idx++] = (float)-10.0;//dumb value
        rawVertexArray[idx++] = (float)-10.0;//dumb value
        rawVertexArray[idx++] = (float)-10.0;//dumb value
    }

    uint* rawJointIndexArray = reinterpret_cast<uint*>(vertexArray.data());
    idx = 0;
    for (int i = 0; i < vertexCount; ++i)
    {
        idx += 12;

        rawJointIndexArray[idx++] = (uint)30;
        rawJointIndexArray[idx++] = (uint)31;
        rawJointIndexArray[idx++] = (uint)32;
        rawJointIndexArray[idx++] = (uint)33;
    }

    //qDebug() << "idx =" << idx;

    QByteArray indexArray;
    indexArray.resize(indicesCount * sizeof(uint));
    uint* rawIndexArray = reinterpret_cast<uint*>(indexArray.data());
    idx = 0;
    for (int i = 0; i < indicesCount; ++i)
    {
        int rawIndex = indices[i];

        if (rawIndex < 0)
        {
            if (i > 0)
            {
                rawIndex = indices[i - 1] + 1;
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "rawIndex less than zero but i == 0";
                rawIndex = 0;
            }
        }

        rawIndexArray[idx++] = (uint)rawIndex;

    }

    vertexBuffer->setData(vertexArray);
    indexBuffer->setData(indexArray);

    Qt3DRender::QAttribute* positionAttribute = new Qt3DRender::QAttribute(geometry);
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    positionAttribute->setDataType(Qt3DRender::QAttribute::Float);
    positionAttribute->setDataSize(3);
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexBuffer);
    positionAttribute->setByteStride(stride);
    positionAttribute->setByteOffset(0);

    Qt3DRender::QAttribute* normalAttribute = new Qt3DRender::QAttribute(geometry);
    normalAttribute->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());
    normalAttribute->setDataType(Qt3DRender::QAttribute::Float);
    normalAttribute->setDataSize(3);
    normalAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    normalAttribute->setBuffer(vertexBuffer);
    normalAttribute->setByteStride(stride);
    normalAttribute->setByteOffset((3) * sizeof(float));

    Qt3DRender::QAttribute* texcoordAttribute = new Qt3DRender::QAttribute(geometry);
    texcoordAttribute->setName(Qt3DRender::QAttribute::defaultTextureCoordinateAttributeName());
    texcoordAttribute->setDataType(Qt3DRender::QAttribute::Float);
    texcoordAttribute->setDataSize(2);
    texcoordAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    texcoordAttribute->setBuffer(vertexBuffer);
    texcoordAttribute->setByteStride(stride);
    texcoordAttribute->setByteOffset((3 + 3) * sizeof(float));

    //qDebug() << Qt3DRender::QAttribute::defaultJointWeightsAttributeName();
    //qDebug() << Qt3DRender::QAttribute::defaultJointIndicesAttributeName();

    Qt3DRender::QAttribute* jointWeightAttribute = new Qt3DRender::QAttribute(geometry);
    jointWeightAttribute->setName(Qt3DRender::QAttribute::defaultJointWeightsAttributeName());
    jointWeightAttribute->setDataType(Qt3DRender::QAttribute::Float);
    jointWeightAttribute->setDataSize(4);
    jointWeightAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    jointWeightAttribute->setBuffer(vertexBuffer);
    jointWeightAttribute->setByteStride(stride);
    jointWeightAttribute->setByteOffset((3 + 3 + 2) * sizeof(float));

    Qt3DRender::QAttribute* jointIndexAttribute = new Qt3DRender::QAttribute(geometry);
    jointIndexAttribute->setName(Qt3DRender::QAttribute::defaultJointIndicesAttributeName());
    jointIndexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedInt);
    jointIndexAttribute->setDataSize(4);
    jointIndexAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    jointIndexAttribute->setBuffer(vertexBuffer);
    jointIndexAttribute->setByteStride(stride);
    jointIndexAttribute->setByteOffset((3 + 3 + 2 + 4) * sizeof(float));

    /*QByteArray vertexJointIndexArray;
    vertexJointIndexArray.resize(vertexCount * sizeof(uint) * 4);
    uint* rawVertexJointIndexArray = reinterpret_cast<uint*>(vertexJointIndexArray.data());
    idx = 0;
    for (int i = 0; i < vertexCount; ++i)
    {
        rawVertexJointIndexArray[idx++] = (uint)3;
        rawVertexJointIndexArray[idx++] = (uint)4;
        rawVertexJointIndexArray[idx++] = (uint)5;
        rawVertexJointIndexArray[idx++] = (uint)6;
    }

    //qDebug() << "idx =" << idx;

    Qt3DRender::QBuffer* vertexJointIndexBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    vertexJointIndexBuffer->setData(vertexJointIndexArray);

    Qt3DRender::QAttribute* jointIndexAttribute = new Qt3DRender::QAttribute(geometry);
    jointIndexAttribute->setName(Qt3DRender::QAttribute::defaultJointIndicesAttributeName());
    jointIndexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedInt);
    jointIndexAttribute->setDataSize(4);
    jointIndexAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    jointIndexAttribute->setBuffer(vertexJointIndexBuffer);
    jointIndexAttribute->setByteStride(4 * sizeof(uint));
    jointIndexAttribute->setByteOffset(0);*/

    Qt3DRender::QAttribute* indexAttribute = new Qt3DRender::QAttribute(geometry);
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedInt);
    indexAttribute->setBuffer(indexBuffer);
    indexAttribute->setCount(indicesCount);

    geometry->addAttribute(positionAttribute);
    geometry->addAttribute(normalAttribute);
    geometry->addAttribute(texcoordAttribute);
    geometry->addAttribute(jointWeightAttribute);
    geometry->addAttribute(jointIndexAttribute);
    geometry->addAttribute(indexAttribute);

    renderer->setGeometry(geometry);
    entity->addComponent(renderer);



    return entity;
}
