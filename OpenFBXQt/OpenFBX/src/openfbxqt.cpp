#include "openfbxqt.h"
#include "ofbx.h"
#include <QFile>
#include <QDebug>
#include <QTransform>
#include <algorithm>

namespace
{

static QMatrix4x4 convertMatrix4x4(const ofbx::Matrix& source)
{
    return QMatrix4x4(
            source.m[0],  source.m[4],  source.m[8],  source.m[12],
            source.m[1],  source.m[5],  source.m[9],  source.m[13],
            source.m[2],  source.m[6],  source.m[10], source.m[14],
            source.m[3],  source.m[7],  source.m[11], source.m[15]);
}

static bool compareJointData(const QPair<GLuint, GLfloat>& joint1, const QPair<GLuint, GLfloat>& joint2)
{
    return joint1.second >= joint2.second;
}

static void loadJoints(const ofbx::Skin* skin, OpenFBXQt::Model* result, QHash<GLuint, QVector<QPair<GLuint, GLfloat>>>& resultJointsData /*QHash<index of vertex, QVector<QPair<joint index, joint weight>>>*/)
{
    if (!result)
    {
        qCritical() << Q_FUNC_INFO << "result is null";
        return;
    }

    if (!skin)
    {
        qCritical() << Q_FUNC_INFO << "skin is null";
        return;
    }

    const int clusterCount = skin->getClusterCount();

    if (clusterCount <= 0)
    {
        qCritical() << Q_FUNC_INFO << "no clusters";
        return;
    }

    int jointIndex = 0;

    QHash<OpenFBXQt::Joint*, const ofbx::Cluster*> clustersByJoints;

    OpenFBXQt::Joint* rootJoint = new OpenFBXQt::Joint("root", jointIndex++, QMatrix4x4(), result);
    clustersByJoints.insert(rootJoint, nullptr);
    result->jointsResultMatrices.append(QMatrix4x4());
    result->joints.append(rootJoint);
    result->jointsByName.insert(rootJoint->name(), rootJoint);
    result->rootJoint = rootJoint;

    QHash<const ofbx::Object*, OpenFBXQt::Joint*> objectsJoints;

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

        OpenFBXQt::Joint* joint = new OpenFBXQt::Joint(object->name, jointIndex++, convertMatrix4x4(cluster->getTransformMatrix()), result);

        clustersByJoints.insert(joint, cluster);
        result->jointsResultMatrices.append(QMatrix4x4());

        objectsJoints.insert(object, joint);
        result->joints.append(joint);
        result->jointsByName.insert(joint->name(), joint);
    }

    for (const ofbx::Object* object : objectsJoints.keys())
    {
        OpenFBXQt::Joint* joint = objectsJoints[object];
        bool addedToParent = false;

        const ofbx::Object* parent = object->getParent();
        if (parent)
        {
            if (objectsJoints.contains(parent))
            {
                OpenFBXQt::Joint* parentJoint = objectsJoints[parent];
                //qDebug() << "added" << joint->name << "to" << parentJoint->name;
                parentJoint->addChild(joint);
                addedToParent = true;
            }
        }

        if (!addedToParent)
        {
            //qDebug() << "added" << joint->name << "to" << rootJoint->name;
            rootJoint->addChild(joint);
        }
    }

    for (OpenFBXQt::Joint* joint : result->joints)
    {
        if (!clustersByJoints.contains(joint))
        {
            qWarning() << Q_FUNC_INFO << "no cluster for joint" << joint->name();
            continue;
        }

        const ofbx::Cluster* cluster = clustersByJoints.value(joint);
        if (!cluster)
        {
            // cluster is null for root
            continue;
        }

        const int weightsCount = cluster->getWeightsCount();
        const int indicesCount = cluster->getIndicesCount();

        if (indicesCount != weightsCount)
        {
            qWarning() << Q_FUNC_INFO << "joint indices count and joint weights count do not match for joint" << joint->name();
            continue;
        }

        const double* weights = cluster->getWeights();
        const int* indices = cluster->getIndices();

        for (int i = 0; i < indicesCount; ++i)
        {
            resultJointsData[indices[i]].append(QPair<GLuint, GLfloat>(joint->index(), weights[i]));
        }
    }

    for (const auto& vertexIndex : resultJointsData.keys())
    {
        std::sort(resultJointsData[vertexIndex].begin(), resultJointsData[vertexIndex].end(), compareJointData);
    }
}

}

OpenFBXQt::Model* OpenFBXQt::load(const QString &fileName)
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
    else if (meshCount > 1)
    {
        qWarning() << Q_FUNC_INFO << "meshes more than 1, only the first one will be loaded ";
    }

    const ofbx::Mesh* mesh = scene->getMesh(0);
    if (!mesh)
    {
        qCritical() << Q_FUNC_INFO << "mesh is null";
        return nullptr;
    }

    const ofbx::Geometry* geometry = mesh->getGeometry();
    if (!geometry)
    {
        qCritical() << Q_FUNC_INFO << "raw geometry is null";
        return nullptr;
    }

    const ofbx::Vec3* positions = geometry->getVertices();
    if (!positions)
    {
        qCritical() << Q_FUNC_INFO << "positions is null";
        return nullptr;
    }

    const int* indices = geometry->getFaceIndices();
    if (!indices)
    {
        qCritical() << Q_FUNC_INFO << "indices is null";
        return nullptr;
    }

    const ofbx::Vec3* normals = geometry->getNormals();
    if (!normals)
    {
        qCritical() << Q_FUNC_INFO << "normals is null";
        return nullptr;
    }

    const ofbx::Vec2* texcoord = geometry->getUVs();
    if (!texcoord)
    {
        qCritical() << Q_FUNC_INFO << "texture coordinats is null";
        return nullptr;
    }

    Model* model = new Model();

    QHash<GLuint, QVector<QPair<GLuint, GLfloat>>> jointsData;

    const ofbx::Skin* skin = geometry->getSkin();
    if (skin)
    {
        loadJoints(skin, model, jointsData);
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "no skin";
    }

    int idx = 0;

    model->addVertexAttribute<GLfloat>("a_position", 3);
    //model->addVertexAttribute<GLfloat>("a_normal", 3);
    model->addVertexAttribute<GLfloat>("a_texcoord", 2);
    model->addVertexAttribute<GLfloat>("a_joint_weights", 4);
    model->addVertexAttribute<GLfloat>("a_joint_indices", 4);

    model->vertexCount = geometry->getVertexCount();
    model->vertexData.resize(model->vertexCount * model->vertexStride);

    GLfloat* rawVertexArray = reinterpret_cast<GLfloat*>(model->vertexData.data());
    idx = 0;
    for (int vertexIndex = 0; vertexIndex < model->vertexCount; ++vertexIndex)
    {
        rawVertexArray[idx++] = (GLfloat)positions[vertexIndex].x;
        rawVertexArray[idx++] = (GLfloat)positions[vertexIndex].y;
        rawVertexArray[idx++] = (GLfloat)positions[vertexIndex].z;

        /*rawVertexArray[idx++] = (GLfloat)normals[vertexIndex].x;
        rawVertexArray[idx++] = (GLfloat)normals[vertexIndex].y;
        rawVertexArray[idx++] = (GLfloat)normals[vertexIndex].z;*/

        rawVertexArray[idx++] = (GLfloat)texcoord[vertexIndex].x;
        rawVertexArray[idx++] = (GLfloat)texcoord[vertexIndex].y;

        if (jointsData.contains(vertexIndex))
        {
            const int jointCountForVertex = jointsData[vertexIndex].count();
            if (jointCountForVertex > 4)
            {
                //qWarning() << Q_FUNC_INFO << "more than 4 joints found for vertex" << vertexIndex << ", joints count =" << jointCountForVertex;
            }

            for (int jointNum = 0; jointNum < qMin(jointCountForVertex, 4); ++jointNum)
            {
                rawVertexArray[idx++] = (GLfloat)jointsData[vertexIndex][jointNum].second;
            }

            for (int jointNum = jointCountForVertex; jointNum < 4; ++jointNum)
            {
                rawVertexArray[idx++] = (GLfloat)0.0;
            }

            for (int jointNum = 0; jointNum < qMin(jointCountForVertex, 4); ++jointNum)
            {
                rawVertexArray[idx++] = (GLfloat)jointsData[vertexIndex][jointNum].first;
            }

            for (int jointNum = jointCountForVertex; jointNum < 4; ++jointNum)
            {
                rawVertexArray[idx++] = (GLfloat)-1;
            }
        }
        else
        {
            rawVertexArray[idx++] = (GLfloat)0.0;
            rawVertexArray[idx++] = (GLfloat)0.0;
            rawVertexArray[idx++] = (GLfloat)0.0;
            rawVertexArray[idx++] = (GLfloat)0.0;

            rawVertexArray[idx++] = (GLfloat)-1;
            rawVertexArray[idx++] = (GLfloat)-1;
            rawVertexArray[idx++] = (GLfloat)-1;
            rawVertexArray[idx++] = (GLfloat)-1;
        }
    }

    model->indexCount = geometry->getIndexCount();
    model->indexData.resize(model->indexCount * sizeof(GLuint));
    GLuint* rawIndexArray = reinterpret_cast<GLuint*>(model->indexData.data());
    idx = 0;
    for (int i = 0; i < model->indexCount; ++i)
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

        rawIndexArray[idx++] = (GLuint)rawIndex;
    }

    scene->destroy();

    model->updateJointsResultMatrices(model->rootJoint, QMatrix4x4());

    return model;
}

template<typename ValueType> void OpenFBXQt::Model::addVertexAttribute(const QString& nameForShader, const int tupleSize)
{
    int offset = 0;

    if (!vertexAttributes.isEmpty())
    {
        const VertexAttributeInfo& last = vertexAttributes.last();
        offset = last.offset + last.tupleSize * sizeof(ValueType);
    }

    VertexAttributeInfo attribute;

    attribute = VertexAttributeInfo();
    attribute.nameForShader = nameForShader;
    attribute.tupleSize = tupleSize;
    attribute.offset = offset;

    vertexStride += tupleSize * sizeof(ValueType);

    vertexAttributes.append(attribute);
}

OpenFBXQt::Model::~Model()
{
    //ToDo: реализовать правильное удаление
}

void OpenFBXQt::Model::updateJointsResultMatrices(Joint* joint, const QMatrix4x4& parentMatrix)
{
    if (!joint)
    {
        qWarning() << Q_FUNC_INFO << "joint is nullptr";
        return;
    }

    QMatrix4x4 matrix = parentMatrix * joint->inverseBindMatrix().inverted() * joint->localTransofrmation() * joint->inverseBindMatrix();

    jointsResultMatrices[joint->index()] = matrix;

    for (Joint* child : qAsConst(joint->children()))
    {
        updateJointsResultMatrices(child, matrix);
    }
}
