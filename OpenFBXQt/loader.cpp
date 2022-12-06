#include "loader.h"
#include "joint.h"
#include "OpenFBX/src/ofbx.h"
#include <QFile>
#include <QTranslator>
#include <QOpenGLFunctions>
#include <QDebug>

namespace ofbxqt
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

QList<Model*> Loader::load(const QString &fileName, QList<Note>& notes)
{
    notes = QList<Note>();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("Failed to open file \"%1\", error: \"%2\"")
                .arg(fileName, file.errorString())));
        return QList<Model*>();
    }

    const QByteArray rawData = file.readAll();
    file.close();

    ofbx::IScene* scene = ofbx::load((ofbx::u8*)rawData.constData(), rawData.size(), (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
    if (!scene)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("No scene")));
        return QList<Model*>();
    }

    const int meshCount = scene->getMeshCount();
    if (meshCount <= 0)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("No meshes in scene")));
        return QList<Model*>();
    }

    QList<Model*> models;
    for (int i = 0; i < meshCount; ++i)
    {
        Model* model = loadMesh(scene->getMesh(i), notes);
        if (model)
        {
            models.append(model);
        }
    }

    scene->destroy();

    return models;
}

void Loader::loadJoints(const ofbx::Skin* skin, ModelData& data,
                       QHash<GLuint, QVector<QPair<GLuint, GLfloat>>>& resultJointsData /*QHash<index of vertex, QVector<QPair<joint index, joint weight>>>*/,
                       QList<Note>& notes)
{
    if (!skin)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("Internal error")));
        qCritical() << Q_FUNC_INFO << "skin is null";
        return;
    }

    const int clusterCount = skin->getClusterCount();

    if (clusterCount <= 0)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("No clusters in skin")));
        return;
    }

    int jointIndex = 0;

    QHash<Joint*, const ofbx::Cluster*> clustersByJoints;

    Joint* rootJoint = new Joint("root", jointIndex++, QMatrix4x4());

    clustersByJoints.insert(rootJoint, nullptr);
    data.skeleton.jointsResultMatrices.append(QMatrix4x4());
    data.skeleton.joints.append(rootJoint);
    data.skeleton.jointsByName.insert(rootJoint->getName(), rootJoint);
    data.skeleton.rootJoint = rootJoint;

    QHash<const ofbx::Object*, Joint*> objectsJoints;

    for (int clusterNum = 0; clusterNum < skin->getClusterCount(); ++clusterNum)
    {
        const ofbx::Cluster* cluster = skin->getCluster(clusterNum);
        if (!cluster)
        {
            notes.append(Note(Note::Type::Warning, QTranslator::tr("Internal error")));
            qWarning() << Q_FUNC_INFO << "cluster is null at index" << clusterNum;
            continue;
        }

        const ofbx::Object* object = cluster->getLink();
        if (!object)
        {
            notes.append(Note(Note::Type::Warning, QTranslator::tr("Internal error")));
            qWarning() << Q_FUNC_INFO << "object/link of cluster is null at index" << clusterNum;
            continue;
        }

        Joint* joint = new Joint(object->name, jointIndex++, convertMatrix4x4(cluster->getTransformMatrix()));

        clustersByJoints.insert(joint, cluster);
        data.skeleton.jointsResultMatrices.append(QMatrix4x4());

        objectsJoints.insert(object, joint);
        data.skeleton.joints.append(joint);

        const QString name = joint->getName();
        if (data.skeleton.jointsByName.contains(name))
        {
            const QString& newName = name + "_1";
            qWarning() << Q_FUNC_INFO << "found joint with already exists name. Joint" << name << "renamed to" << newName;
            data.skeleton.jointsByName.insert(newName, joint);
        }
        else
        {
            data.skeleton.jointsByName.insert(joint->getName(), joint);
        }
    }

    const auto keys = objectsJoints.keys();
    for (const ofbx::Object* object : qAsConst(keys))
    {
        Joint* joint = objectsJoints[object];
        bool addedToParent = false;

        const ofbx::Object* parent = object->getParent();
        if (parent)
        {
            if (objectsJoints.contains(parent))
            {
                Joint* parentJoint = objectsJoints[parent];
                parentJoint->addChild(joint);
                addedToParent = true;
            }
        }

        if (!addedToParent)
        {
            rootJoint->addChild(joint);
        }
    }

    for (Joint* joint : qAsConst(data.skeleton.joints))
    {
        if (!clustersByJoints.contains(joint))
        {
            notes.append(Note(Note::Type::Warning, QTranslator::tr("No cluster for joint \"%1\"").arg(joint->getName())));
            qWarning() << Q_FUNC_INFO << QString("no cluster for joint \"%1\"").arg(joint->getName());
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
            notes.append(Note(Note::Type::Warning, QTranslator::tr("Joint indices count and joint weights count do not match for joint \"%1\"").arg(joint->getName())));
            qWarning() << Q_FUNC_INFO << QString("joint indices count and joint weights count do not match for joint \"%1\"").arg(joint->getName());
            continue;
        }

        const double* weights = cluster->getWeights();
        const int* indices = cluster->getIndices();

        for (int i = 0; i < indicesCount; ++i)
        {
            resultJointsData[indices[i]].append(QPair<GLuint, GLfloat>(joint->index, weights[i]));
        }
    }

    {
        const auto keys = resultJointsData.keys();
        for (const auto& vertexIndex : keys)
        {
            std::sort(resultJointsData[vertexIndex].begin(), resultJointsData[vertexIndex].end(), compareJointData);
        }
    }
}

Model *Loader::loadMesh(const ofbx::Mesh *mesh, QList<Note>& notes)
{
    if (!mesh)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("Internal error")));
        qCritical() << Q_FUNC_INFO << "mesh is null";
        return nullptr;
    }

    const ofbx::Geometry* geometry = mesh->getGeometry();
    if (!geometry)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("Internal error")));
        qCritical() << Q_FUNC_INFO << "geometry is null";
        return nullptr;
    }

    const ofbx::Vec3* positions = geometry->getVertices();
    if (!positions)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("Internal error")));
        qCritical() << Q_FUNC_INFO << "positions is null";
        return nullptr;
    }

    const int* indices = geometry->getFaceIndices();
    if (!indices)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("Internal error")));
        qCritical() << Q_FUNC_INFO << "indices is null";
        return nullptr;
    }

    const ofbx::Vec3* normals = geometry->getNormals();
    if (!normals)
    {
        notes.append(Note(Note::Type::Error, QTranslator::tr("Internal error")));
        qCritical() << Q_FUNC_INFO << "normals is null";
        return nullptr;
    }

    ModelData* data = new ModelData();

    data->sourceMatrix = convertMatrix4x4(mesh->getLocalTransform());

    QHash<GLuint, QVector<QPair<GLuint, GLfloat>>> jointsData;

    const ofbx::Skin* skin = geometry->getSkin();
    if (skin)
    {
        loadJoints(skin, *data, jointsData, notes);
    }

    int idx = 0;

    addVertexAttributeGLfloat(*data, "a_position", 3);
    addVertexAttributeGLfloat(*data, "a_normal", 3);

    const ofbx::Vec2* texcoord = geometry->getUVs();
    if (texcoord)
    {
        addVertexAttributeGLfloat(*data, "a_texcoord", 2);
    }

    if (data->skeleton.getJoints().count() > 0)
    {
        addVertexAttributeGLfloat(*data, "a_joint_weights", 4);
        addVertexAttributeGLfloat(*data, "a_joint_indices", 4);
    }

    data->vertexCount = geometry->getVertexCount();
    data->vertexData.resize(data->vertexCount * data->vertexStride);

    bool foundTooMuchJoints = false;

    GLfloat* rawVertexArray = reinterpret_cast<GLfloat*>(data->vertexData.data());
    idx = 0;
    for (int vertexIndex = 0; vertexIndex < data->vertexCount; ++vertexIndex)
    {
        rawVertexArray[idx++] = (GLfloat)positions[vertexIndex].x;
        rawVertexArray[idx++] = (GLfloat)positions[vertexIndex].y;
        rawVertexArray[idx++] = (GLfloat)positions[vertexIndex].z;

        rawVertexArray[idx++] = (GLfloat)normals[vertexIndex].x;
        rawVertexArray[idx++] = (GLfloat)normals[vertexIndex].y;
        rawVertexArray[idx++] = (GLfloat)normals[vertexIndex].z;

        if (texcoord)
        {
            rawVertexArray[idx++] = (GLfloat)texcoord[vertexIndex].x;
            rawVertexArray[idx++] = (GLfloat)texcoord[vertexIndex].y;
        }

        if (data->skeleton.getJoints().count() > 0)
        {
            if (jointsData.contains(vertexIndex))
            {
                const int jointCountForVertex = jointsData[vertexIndex].count();
                if (jointCountForVertex > 4)
                {
                    foundTooMuchJoints = true;
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
    }

    if (foundTooMuchJoints)
    {
        notes.append(Note(Note::Type::Warning, QTranslator::tr("More than 4 joints not supported. Extra joints will be ignored")));
        qWarning() << Q_FUNC_INFO << "more than 4 joints not supported. Extra joints will be ignored";
    }

    data->indexCount = geometry->getIndexCount();
    data->indexData.resize(data->indexCount * sizeof(GLuint));
    GLuint* rawIndexArray = reinterpret_cast<GLuint*>(data->indexData.data());
    idx = 0;

    bool foundLessThenZero = false;
    for (int i = 0; i < data->indexCount; ++i)
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
                foundLessThenZero = true;
                rawIndex = 0;
            }
        }

        rawIndexArray[idx++] = (GLuint)rawIndex;
    }

    if (foundLessThenZero)
    {
        notes.append(Note(Note::Type::Warning, QTranslator::tr("Internal error")));
        qWarning() << Q_FUNC_INFO << "rawIndex less than zero but i == 0";
    }

    data->skeleton.update();

    ModelDataStorage::data.append(data);

    return new Model(*data);
}

void Loader::addVertexAttributeGLfloat(ModelData& modelData, const QString &nameForShader, const int tupleSize)
{
    int offset = 0;

    if (!modelData.vertexAttributes.isEmpty())
    {
        const VertexAttributeInfo& last = modelData.vertexAttributes.last();
        offset = last.offset + last.tupleSize * sizeof(GLfloat);
    }

    VertexAttributeInfo attribute;

    attribute = VertexAttributeInfo();
    attribute.nameForShader = nameForShader;
    attribute.tupleSize = tupleSize;
    attribute.offset = offset;

    modelData.vertexStride += tupleSize * sizeof(GLfloat);

    modelData.vertexAttributes.append(attribute);
}

}
