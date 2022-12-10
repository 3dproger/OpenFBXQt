#include "loader.h"
#include "joint.h"
#include "OpenFBX/src/ofbx.h"
#include <QFile>
#include <QTranslator>
#include <QOpenGLFunctions>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

namespace ofbxqt
{

static bool shuffledSpareColor = false;
static int currentSpareColors = 0;

static QList<QColor> spareColors =
{
    QColor(239, 154, 154),
    QColor(244, 67, 54),
    QColor(183, 28, 28),
    QColor(206, 147, 216),
    QColor(156, 39, 176),
    QColor(74, 20, 140),
    QColor(179, 157, 219),
    QColor(103, 58, 183),
    QColor(49, 27, 146),
    QColor(159, 168, 218),
    QColor(33, 150, 243),
    QColor(13, 71, 161),
    QColor(129, 212, 250),
    QColor(3, 169, 244),
    QColor(1, 87, 155),
    QColor(128, 222, 234),
    QColor(0, 188, 212),
    QColor(0, 96, 100),
    QColor(128, 203, 196),
    QColor(0, 150, 136),
    QColor(0, 77, 64),
    QColor(165, 214, 167),
    QColor(76, 175, 80),
    QColor(27, 94, 32),
    QColor(197, 225, 165),
    QColor(139, 195, 74),
    QColor(51, 105, 30),
    QColor(255, 245, 157),
    QColor(255, 235, 59),
    QColor(245, 127, 23),
    QColor(255, 224, 130),
    QColor(255, 193, 7),
    QColor(255, 111, 0),
    QColor(255, 204, 128),
    QColor(255, 87, 34),
    QColor(191, 54, 12),
    QColor(188, 170, 164),
    QColor(121, 85, 72),
    QColor(62, 39, 35),
    QColor(238, 238, 238),
    QColor(158, 158, 158),
    QColor(33, 33, 33),
    QColor(176, 190, 197),
    QColor(96, 125, 139),
    QColor(38, 50, 56),
};

static QMatrix4x4 convertMatrix4x4(const ofbx::Matrix& source)
{
    return QMatrix4x4(
            source.m[0],  source.m[4],  source.m[8],  source.m[12],
            source.m[1],  source.m[5],  source.m[9],  source.m[13],
            source.m[2],  source.m[6],  source.m[10], source.m[14],
            source.m[3],  source.m[7],  source.m[11], source.m[15]);
}

static QString convertString2048(const ofbx::DataView& dataView)
{
    static const int MaxSize = 2048;
    char chars[MaxSize];
    dataView.toString(chars);
    return QString(chars);
}

static QColor covertColor(const ofbx::Color& color)
{
    return QColor::fromRgbF(color.r, color.g, color.b);
}

static QString textureTypeToString(const ofbx::Texture::TextureType type)
{
    switch (type)
    {
    case ofbx::Texture::DIFFUSE: return "DIFFUSE";
    case ofbx::Texture::NORMAL: return "NORMAL";
    case ofbx::Texture::SPECULAR: return "SPECULAR";
    case ofbx::Texture::SHININESS: return "SHININESS";
    case ofbx::Texture::AMBIENT: return "AMBIENT";
    case ofbx::Texture::EMISSIVE: return "EMISSIVE";
    case ofbx::Texture::REFLECTION: return "REFLECTION";
    case ofbx::Texture::COUNT: return "<COUNT>";
    }

    return "<UNKNOWN>";
}

static bool compareJointData(const QPair<GLuint, GLfloat>& joint1, const QPair<GLuint, GLfloat>& joint2)
{
    return joint1.second >= joint2.second;
}

static bool isCompatibleAxisDirection(const ModelData::AxisDirection a, const ModelData::AxisDirection b)
{
    if (a == b)
    {
        return false;
    }

    if ((a == ModelData::AxisDirection::XPlus && b == ModelData::AxisDirection::XMinus) ||
        (b == ModelData::AxisDirection::XPlus && a == ModelData::AxisDirection::XMinus))
    {
        return false;
    }

    if ((a == ModelData::AxisDirection::YPlus && b == ModelData::AxisDirection::YMinus) ||
        (b == ModelData::AxisDirection::YPlus && a == ModelData::AxisDirection::YMinus))
    {
        return false;
    }

    if ((a == ModelData::AxisDirection::ZPlus && b == ModelData::AxisDirection::ZMinus) ||
        (b == ModelData::AxisDirection::ZPlus && a == ModelData::AxisDirection::ZMinus))
    {
        return false;
    }

    return true;
}

Loader::Loader()
{
    if (!shuffledSpareColor)
    {
        shuffledSpareColor = true;
        std::random_shuffle(spareColors.begin(), spareColors.end());
    }
}

FileInfo Loader::open(const QString &fileName, const OpenModelConfig config_)
{
    config = config_;

    fileInfo = FileInfo();
    fileInfo.absoluteFileName = fileName;
    fileInfo.fileName = QFileInfo(fileName).fileName();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        addNote(Note::Type::Error, QTranslator::tr("Failed to open file \"%1\", error: \"%2\"").arg(fileName, file.errorString()));
        qCritical() << Q_FUNC_INFO << "failed to open file" << fileName << ", error:" << file.errorString();
        return fileInfo;
    }

    const QByteArray rawData = file.readAll();
    file.close();

    ofbx::IScene* scene = ofbx::load((ofbx::u8*)rawData.constData(), rawData.size(), (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
    if (!scene)
    {
        addNote(Note::Type::Error, QTranslator::tr("No scene"));
        qCritical() << Q_FUNC_INFO << "no scene";
        return fileInfo;
    }

    // TODO: need to add the ability to change the direction of the axes for the scene or software
    // Blender: up = Z+, forward = Y+
    // Unity: up = Y+, forward = Z+

    upDirection = ModelData::DefaultUpDirection;
    forwardDirection = ModelData::DefaultForwardDirection;
    const ofbx::GlobalSettings* settings = scene->getGlobalSettings();
    if (settings)
    {
        convertAxisDirection(upDirection, settings->UpAxis, settings->UpAxisSign);

        // Note from OpenFBX (ofbx.h file) about forward axis:
        // this seems to be 1-2 in Autodesk (odd/even parity), and 0-2 in Blender (axis as in UpAxis)
        // I recommend to ignore FrontAxis and use just UpVector
        // TODO: solve this problem. Possibly adjust accordingly
        convertAxisDirection(forwardDirection, settings->FrontAxis, settings->FrontAxisSign);

        if (!isCompatibleAxisDirection(upDirection, forwardDirection))
        {
            addNote(Note::Type::Error, QTranslator::tr("Incompatible axis directions %1 and %2. Will use default").arg(ModelData::axisDirectionToString(upDirection), ModelData::axisDirectionToString(forwardDirection)));
            qCritical() << Q_FUNC_INFO << "incompatible axis directions" << ModelData::axisDirectionToString(upDirection) << "and" << ModelData::axisDirectionToString(forwardDirection) << ". Will use default";

            upDirection = ModelData::DefaultUpDirection;
            forwardDirection = ModelData::DefaultForwardDirection;
        }
    }
    else
    {
        addNote(Note::Type::Error, QTranslator::tr("No file global settings"));
        qCritical() << Q_FUNC_INFO << "No file global settings";
    }

    const int meshCount = scene->getMeshCount();
    if (meshCount <= 0)
    {
        scene->destroy();
        addNote(Note::Type::Error, QTranslator::tr("No meshes in scene"));
        qCritical() << Q_FUNC_INFO << "no meshes in scene";
        return fileInfo;
    }

    const QString absoluteDirectoryPath = QFileInfo(fileName).absoluteDir().absolutePath();

    QVector<QPair<const ofbx::Mesh*, std::shared_ptr<Model>>> modelBinds;

    QVector<std::shared_ptr<Model>> allModels;
    for (int i = 0; i < meshCount; ++i)
    {
        const ofbx::Mesh* mesh = scene->getMesh(i);
        std::shared_ptr<Model> model = loadMesh(mesh, i, absoluteDirectoryPath);
        if (model)
        {
            allModels.append(model);
        }

        modelBinds.append(QPair<const ofbx::Mesh*, std::shared_ptr<Model>>(mesh, model ? model : nullptr));
    }

    for (int i = 0; i < modelBinds.count(); ++i)
    {
        for (int j = 0; j < modelBinds.count(); ++j)
        {
            if (i == j || !modelBinds[i].first || !modelBinds[i].second || !modelBinds[j].first || !modelBinds[j].second
                    || !modelBinds[i].second->parent.expired() || !modelBinds[j].second->parent.expired())
            {
                continue;
            }

            std::shared_ptr<Model> model1 = modelBinds[i].second;
            std::shared_ptr<Model> model2 = modelBinds[j].second;

            const ofbx::Mesh* mesh1 = modelBinds[i].first;
            const ofbx::Mesh* mesh2 = modelBinds[j].first;

            const ofbx::Object* parent1 = mesh1->getParent();
            const ofbx::Object* parent2 = mesh1->getParent();

            if (parent1 && parent1 == (const ofbx::Object*)mesh2)
            {
                model1->parent = model2;
                model2->children.push_back(model1);
            }
            else if (parent2 && parent2 == (const ofbx::Object*)mesh1)
            {
                model2->parent = model1;
                model1->children.push_back(model2);
            }
        }
    }

    for (const std::shared_ptr<Model>& model : allModels)
    {
        if (model->parent.expired())
        {
            fileInfo.topLevelModels.append(model);
        }
    }

    scene->destroy();

    return fileInfo;
}

void Loader::addNote(const Note::Type type, const QString &text)
{
    fileInfo.notes.append(Note(type, text));
}

void Loader::loadJoints(const ofbx::Skin* skin, ModelData& data, QHash<GLuint, QVector<QPair<GLuint, GLfloat>>>& resultJointsData)
{
    if (!skin)
    {
        addNote(Note::Type::Error, QTranslator::tr("Internal error"));
        qCritical() << Q_FUNC_INFO << "skin is null";
        return;
    }

    const int clusterCount = skin->getClusterCount();
    if (clusterCount <= 0)
    {
        addNote(Note::Type::Error, QTranslator::tr("No clusters in skin"));
        qCritical() << Q_FUNC_INFO << "no clusters in skin";
        return;
    }

    data.armature = std::shared_ptr<Armature>(new Armature());

    int jointIndex = 0;

    std::map<std::shared_ptr<Joint>, const ofbx::Cluster*> clustersByJoints;

    QHash<const ofbx::Object*, std::shared_ptr<Joint>> objectsJoints;

    for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
    {
        const ofbx::Cluster* cluster = skin->getCluster(clusterIndex);
        if (!cluster)
        {
            addNote(Note::Type::Error, QTranslator::tr("Internal error"));
            qCritical() << Q_FUNC_INFO << "cluster is null at index" << clusterIndex;
            continue;
        }

        const ofbx::Object* object = cluster->getLink();
        if (!object)
        {
            addNote(Note::Type::Error, QTranslator::tr("Internal error"));
            qCritical() << Q_FUNC_INFO << "link of cluster" << clusterIndex << "is null";
            continue;
        }

        std::shared_ptr<Joint> joint = std::shared_ptr<Joint>(new Joint(object->name, jointIndex++, convertMatrix4x4(cluster->getTransformMatrix())));
        joint->armature = data.armature;

        clustersByJoints[joint] = cluster;
        data.armature->jointsResultMatrices.append(QMatrix4x4());

        objectsJoints.insert(object, joint);

        QString name = joint->getName();
        if (data.armature->jointsByName.contains(name))
        {
            const QString newName = name + "_1";

            addNote(Note::Type::Warning, QTranslator::tr("Found a joint with an already existing name. Joint \"%1\" renamed to \"%2\"")
                    .arg(name, newName));

            qWarning() << Q_FUNC_INFO << "found a joint with an already existing name. Joint" << name << "renamed to" << newName;

            name = newName;
        }

        data.armature->jointsByName.insert(name, data.armature->allJoints.count());
        data.armature->allJoints.append(joint);
    }

    const auto keys = objectsJoints.keys();
    for (const ofbx::Object* object : qAsConst(keys))
    {
        std::shared_ptr<Joint> joint = objectsJoints[object];
        bool addedToParent = false;

        const ofbx::Object* parent = object->getParent();
        if (parent)
        {
            if (objectsJoints.contains(parent))
            {
                std::shared_ptr<Joint> parentJoint = objectsJoints[parent];
                parentJoint->addChild(joint);
                joint->parent = parentJoint;
                addedToParent = true;
            }
        }

        if (!addedToParent)
        {
            data.armature->topLevelJoints.append(joint);
        }
    }

    static const int MaxJointsSupported = 100;
    if (data.armature->allJoints.count() > MaxJointsSupported)
    {
        addNote(Note::Type::Error, QTranslator::tr("No more than %1 joints supported, found %2")
                .arg(MaxJointsSupported).arg(data.armature->allJoints.count()));
        qCritical() << Q_FUNC_INFO << "no more than" << MaxJointsSupported << "supported, found" << data.armature->allJoints.count();
    }

    for (const std::shared_ptr<Joint> &joint : qAsConst(data.armature->allJoints))
    {
        if (clustersByJoints.find(joint) == clustersByJoints.end())
        {
            addNote(Note::Type::Error, QTranslator::tr("No cluster for joint \"%1\"").arg(joint->getName()));
            qCritical() << Q_FUNC_INFO << QString("no cluster for joint \"%1\"").arg(joint->getName());
            continue;
        }

        const ofbx::Cluster* cluster = clustersByJoints[joint];
        if (!cluster)
        {
            // cluster is null for root
            continue;
        }

        const int weightsCount = cluster->getWeightsCount();
        const int indicesCount = cluster->getIndicesCount();

        if (indicesCount != weightsCount)
        {
            addNote(Note::Type::Error, QTranslator::tr("Joint indices count and joint weights count do not match for joint \"%1\"").arg(joint->getName()));
            qCritical() << Q_FUNC_INFO << QString("joint indices count and joint weights count do not match for joint \"%1\"").arg(joint->getName());
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

std::shared_ptr<Model> Loader::loadMesh(const ofbx::Mesh *mesh, const int meshIndex, const QString& absoluteDirectoryPath)
{
    if (!mesh)
    {
        addNote(Note::Type::Error, QTranslator::tr("Internal error"));
        qCritical() << Q_FUNC_INFO << "mesh is null. Mesh" << meshIndex;
        return nullptr;
    }

    const ofbx::Geometry* geometry = mesh->getGeometry();
    if (!geometry)
    {
        addNote(Note::Type::Error, QTranslator::tr("No geometry. Mesh %1").arg(meshIndex));
        qCritical() << Q_FUNC_INFO << "no geometry. Mesh" << meshIndex;
        return nullptr;
    }

    const ofbx::Vec3* positions = geometry->getVertices();
    if (!positions)
    {
        addNote(Note::Type::Error, QTranslator::tr("No positions. Mesh %1").arg(meshIndex));
        qCritical() << Q_FUNC_INFO << "no positions. Mesh" << meshIndex;
        return nullptr;
    }

    const int* indices = geometry->getFaceIndices();
    if (!indices)
    {
        addNote(Note::Type::Error, QTranslator::tr("No indices. Mesh %1").arg(meshIndex));
        qCritical() << Q_FUNC_INFO << "no indices. Mesh" << meshIndex;
        return nullptr;
    }

    const ofbx::Vec3* normals = geometry->getNormals();
    if (!normals)
    {
        addNote(Note::Type::Error, QTranslator::tr("No normals. Mesh %1").arg(meshIndex));
        qCritical() << Q_FUNC_INFO << "no normals. Mesh" << meshIndex;
        return nullptr;
    }

    std::shared_ptr<Material> material = std::shared_ptr<Material>(new Material());

    if (config.loadMaterial)
    {
        const int materialsCount = mesh->getMaterialCount();
        if (materialsCount > 1)
        {
            addNote(Note::Type::Warning, QTranslator::tr("Only one material supported but found %1. Mesh %2")
                              .arg(materialsCount).arg(meshIndex));
            qWarning() << Q_FUNC_INFO << "only one material supported but found" << materialsCount << ". Mesh" << meshIndex;
        }

        if (materialsCount > 0)
        {
            loadMaterial(mesh->getMaterial(0), material, meshIndex, 0, absoluteDirectoryPath);
        }
        else
        {
            addNote(Note::Type::Warning, QTranslator::tr("No materials. Mesh %1").arg(meshIndex));
            qWarning() << Q_FUNC_INFO << "no materials. Mesh" << meshIndex;
        }
    }

    if (!material->diffuseColor)
    {
        QColor* color = new QColor(191, 191, 191);

        if (!spareColors.isEmpty())
        {
            if (currentSpareColors >= spareColors.count())
            {
                currentSpareColors = 0;
            }

            *color = spareColors[currentSpareColors];

            currentSpareColors++;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "spare colors is empty. Used default color";
        }

        material->diffuseColor = std::unique_ptr<QColor>(color);
    }

    std::shared_ptr<ModelData> data(new ModelData());

    data->name = QString(mesh->name);

    data->material = material;
    data->sourceMatrix = QMatrix4x4();

    switch (upDirection)
    {
    case ofbxqt::ModelData::AxisDirection::XPlus:
        data->sourceMatrix.rotate(90, QVector3D(0, 1, 0));
        break;
    case ofbxqt::ModelData::AxisDirection::XMinus:
        data->sourceMatrix.rotate(90, QVector3D(0, -1, 0));
        break;
    case ofbxqt::ModelData::AxisDirection::YPlus:
        data->sourceMatrix.rotate(90, QVector3D(1, 0, 0));
        break;
    case ofbxqt::ModelData::AxisDirection::YMinus:
        data->sourceMatrix.rotate(90, QVector3D(-1, 0, 0));
        break;
    case ofbxqt::ModelData::AxisDirection::ZPlus:
        data->sourceMatrix.scale(1, 1, 1);
        break;
    case ofbxqt::ModelData::AxisDirection::ZMinus:
        data->sourceMatrix.scale(1, 1, -1);
        break;
    }

    //qDebug() << "up =" << ModelData::axisDirectionToString(upDirection);

    /*switch (forwardDirection) // TODO: fix
    {
    case ofbxqt::ModelData::AxisDirection::XPlus:
        // TODO
        break;
    case ofbxqt::ModelData::AxisDirection::XMinus:
        // TODO
        break;
    case ofbxqt::ModelData::AxisDirection::YPlus:
        data->sourceMatrix.scale(1, 1, 1);
        break;
    case ofbxqt::ModelData::AxisDirection::YMinus:
        data->sourceMatrix.scale(1, -1, 1);
        break;
    case ofbxqt::ModelData::AxisDirection::ZPlus:
        // TODO
        break;
    case ofbxqt::ModelData::AxisDirection::ZMinus:
        // TODO
        break;
    }

    qDebug() << "forward =" << ModelData::axisDirectionToString(forwardDirection);*/

    if (config.loadTransform)
    {
        //data->sourceMatrix *= convertMatrix4x4(mesh->getLocalTransform());
        data->sourceMatrix *= convertMatrix4x4(mesh->getGlobalTransform());
        const ofbx::Pose* pose = mesh->getPose();
        if (pose)
        {
            //data->sourceMatrix *= convertMatrix4x4(pose->getMatrix());
        }
    }

    QHash<GLuint, QVector<QPair<GLuint, GLfloat>>> jointsData; // <vertex index, QVector<QPair<joint index, joint weight>>>

    if (config.loadArmature)
    {
        const ofbx::Skin* skin = geometry->getSkin();
        if (skin)
        {
            loadJoints(skin, *data, jointsData);
        }
    }

    int idx = 0;

    addVertexAttributeGLfloat(*data, "a_position", 3);
    addVertexAttributeGLfloat(*data, "a_normal", 3);

    const ofbx::Vec2* texcoord = geometry->getUVs();
    if (texcoord)
    {
        addVertexAttributeGLfloat(*data, "a_texcoord", 2);
    }

    if (data->armature)
    {
        addVertexAttributeGLfloat(*data, "a_joint_weights", 4);
        addVertexAttributeGLfloat(*data, "a_joint_indices", 4);
    }

    data->vertexCount = geometry->getVertexCount();
    data->vertexData.resize(data->vertexCount * data->vertexStride);

    static const int MaxJointsForVertex = 4;
    int foundTooMuchJointsCount = -1;

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

        if (data->armature)
        {
            int jointCountForVertex = 0;

            if (jointsData.contains(vertexIndex))
            {
                jointCountForVertex = jointsData[vertexIndex].count();
                if (jointCountForVertex > MaxJointsForVertex)
                {
                    if (jointCountForVertex > foundTooMuchJointsCount)
                    {
                        foundTooMuchJointsCount = jointCountForVertex;
                    }
                }
            }

            for (int jointIndex = 0; jointIndex < MaxJointsForVertex; ++jointIndex)
            {
                if (jointIndex < jointCountForVertex)
                {
                    rawVertexArray[idx++] = (GLfloat)jointsData[vertexIndex][jointIndex].second;
                }
                else
                {
                    rawVertexArray[idx++] = (GLfloat)-1;
                }
            }

            for (int jointIndex = 0; jointIndex < MaxJointsForVertex; ++jointIndex)
            {
                if (jointIndex < jointCountForVertex)
                {
                    rawVertexArray[idx++] = (GLfloat)jointsData[vertexIndex][jointIndex].first;
                }
                else
                {
                    rawVertexArray[idx++] = (GLfloat)-1;
                }
            }
        }
    }

    if (foundTooMuchJointsCount != -1)
    {
        addNote(Note::Type::Warning, QTranslator::tr("More than %1 joint weights per vertex not supported, found %2. Extra weights will be ignored. Mesh %3")
                          .arg(MaxJointsForVertex).arg(foundTooMuchJointsCount).arg(meshIndex));
        qWarning() << Q_FUNC_INFO << "more than" << MaxJointsForVertex << "joint weights per vertex not supported, found" << foundTooMuchJointsCount << ". Extra weights will be ignored, mesh" << meshIndex;
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
        addNote(Note::Type::Warning, QTranslator::tr("Internal error"));
        qWarning() << Q_FUNC_INFO << "rawIndex less than zero but i == 0";
    }

    DataStorage::data.append(data);
    std::shared_ptr<Model> model(new Model(data));

    if (data->armature)
    {
        data->armature->model = model;
        data->armature->update();
        model->armature = data->armature;
    }

    return model;
}

void Loader::loadMaterial(const ofbx::Material *rawMaterial, std::shared_ptr<Material> material, const int meshIndex, const int materialIndex, const QString& absoluteDirectoryPath)
{
    if (!rawMaterial)
    {
        addNote(Note::Type::Error, QTranslator::tr("Internal error"));
        qCritical() << Q_FUNC_INFO << "raw material is null, mesh index =" << meshIndex << ", material index =" << materialIndex;
        return;
    }

    if (!material)
    {
        addNote(Note::Type::Error, QTranslator::tr("Internal error"));
        qCritical() << Q_FUNC_INFO << "material is null, mesh index =" << meshIndex << ", material index =" << materialIndex;
        return;
    }

    for (int i = 0; i < (int)ofbx::Texture::TextureType::COUNT; ++i)
    {
        const ofbx::Texture::TextureType type = (ofbx::Texture::TextureType)i;
        const QString textureTypeStr = textureTypeToString(type);

        const ofbx::Texture* rawTexture = rawMaterial->getTexture(type);
        if (rawTexture)
        {
            bool isSupportedTextureType = false;
            switch (type)
            {
            case ofbx::Texture::DIFFUSE:
                isSupportedTextureType = true;
                if (config.loadDiffuseTexture)
                {
                    material->diffuseTexture = loadTexture(rawTexture, absoluteDirectoryPath, meshIndex, materialIndex, type);
                }
                break;

            case ofbx::Texture::NORMAL:
            case ofbx::Texture::SPECULAR:
            case ofbx::Texture::SHININESS:
            case ofbx::Texture::AMBIENT:
            case ofbx::Texture::EMISSIVE:
            case ofbx::Texture::REFLECTION:
            case ofbx::Texture::COUNT:
                break;
            }

            if (!isSupportedTextureType)
            {
                addNote(Note::Type::Warning, QTranslator::tr("Texture type \"%1\" not supported. Mesh %2, material %3")
                                  .arg(textureTypeStr).arg(meshIndex).arg(materialIndex));
                qWarning() << Q_FUNC_INFO << "texture type" << textureTypeStr << "not supported";
            }
        }
    }

    if (config.loadDiffuseColor)
    {
        material->diffuseColor = std::unique_ptr<QColor>(new QColor(covertColor(rawMaterial->getDiffuseColor())));
    }
}

std::shared_ptr<TextureInfo> Loader::loadTexture(const ofbx::Texture* rawTexture, const QString &absoluteDirectoryPath, const int meshIndex, const int materialIndex, ofbx::Texture::TextureType type)
{
    const QString textureTypeStr = textureTypeToString(type);
    if (!rawTexture)
    {
        addNote(Note::Type::Error, QTranslator::tr("Internal error"));
        qCritical() << Q_FUNC_INFO << "raw texture is null, mesh index =" << meshIndex << ", material index =" << materialIndex << ", texture type =" << textureTypeStr;
        return nullptr;
    }

    //TODO: implement search in relative directory path
    const QString rawRelativeFileName = convertString2048(rawTexture->getRelativeFileName());
    const QString relativeFileName = QFileInfo(rawRelativeFileName).fileName();

    if (relativeFileName.isEmpty())
    {
        addNote(Note::Type::Error, QTranslator::tr("Empty texture image relative file name. Mesh %1, material %2, texture %3")
                          .arg(meshIndex).arg(materialIndex).arg(textureTypeStr));
        qCritical() << Q_FUNC_INFO << "empty texture image relative file name. Mesh " << meshIndex << ", material" << materialIndex << ", texture" << textureTypeStr;
        return nullptr;
    }

    const QString fileName = absoluteDirectoryPath + "/" + relativeFileName;

    if (rawRelativeFileName != relativeFileName)
    {
        addNote(Note::Type::Info, QTranslator::tr("Path \"%1\" converted to \"%2\". Mesh %3, material %4, texture %5")
                          .arg(rawRelativeFileName, fileName).arg(meshIndex).arg(materialIndex).arg(textureTypeStr));
    }

    const QFileInfo fileInfo(fileName);
    if (!fileInfo.exists())
    {
        addNote(Note::Type::Error, QTranslator::tr("File \"%1\" not found. Mesh %2, material %3, texture %4")
                          .arg(fileName).arg(meshIndex).arg(materialIndex).arg(textureTypeStr));
        qCritical() << Q_FUNC_INFO << "file" << fileName << "not found. Mesh " << meshIndex << ", material" << materialIndex << ", texture" << textureTypeStr;
        return nullptr;
    }

    if (!fileInfo.isReadable())
    {
        addNote(Note::Type::Error, QTranslator::tr("File \"%1\" not readable. Mesh %2, material %3, texture %4")
                          .arg(fileName).arg(meshIndex).arg(materialIndex).arg(textureTypeStr));
        qCritical() << Q_FUNC_INFO << "file" << fileName << "not readable. Mesh " << meshIndex << ", material" << materialIndex << ", texture" << textureTypeStr;
        return nullptr;
    }

    const auto it = DataStorage::textures.find(fileName);
    if (it != DataStorage::textures.end())
    {
        return it->second;
    }

    QImage image(fileName);
    if (image.isNull())
    {
        addNote(Note::Type::Error, QTranslator::tr("Failed to open image \"%1\". Mesh %2, material %3, texture %4")
                          .arg(fileName).arg(meshIndex).arg(materialIndex).arg(textureTypeStr));
        qCritical() << Q_FUNC_INFO << "failed to open image" << fileName << ". Mesh " << meshIndex << ", material" << materialIndex << ", texture" << textureTypeStr;
        return nullptr;
    }

    std::shared_ptr<TextureInfo> texture = std::shared_ptr<TextureInfo>(new TextureInfo(image, fileName));
    DataStorage::textures[fileName] = texture;

    addNote(Note::Type::Info, QTranslator::tr("Opened %1 texture \"%2\". Mesh %3, material %4, texture %5")
                      .arg(textureTypeStr, fileName).arg(meshIndex).arg(materialIndex).arg(textureTypeStr));

    return texture;
}

void Loader::addVertexAttributeGLfloat(ModelData& data, const QString &nameForShader, const int tupleSize)
{
    int offset = 0;

    if (!data.vertexAttributes.isEmpty())
    {
        const VertexAttributeInfo& last = data.vertexAttributes.last();
        offset = last.offset + last.tupleSize * sizeof(GLfloat);
    }

    VertexAttributeInfo attribute;

    attribute = VertexAttributeInfo();
    attribute.nameForShader = nameForShader;
    attribute.tupleSize = tupleSize;
    attribute.offset = offset;

    data.vertexStride += tupleSize * sizeof(GLfloat);

    data.vertexAttributes.append(attribute);
}

void Loader::convertAxisDirection(ModelData::AxisDirection& value, const int axis, const int sign)
{
    if (axis != 0 && axis != 1 && axis != 2)
    {
        addNote(Note::Type::Error, QTranslator::tr("Wrong axis value %1").arg(axis));
        qCritical() << Q_FUNC_INFO << "wrong axis value" << axis;
        return;
    }

    if (sign != 1 && sign != -1)
    {
        addNote(Note::Type::Error, QTranslator::tr("Wrong axis sign %1").arg(sign));
        qCritical() << Q_FUNC_INFO << "wrong axis sign" << sign;
        return;
    }

    switch (axis)
    {
    case 0:
        value = sign > 0 ? ModelData::AxisDirection::XPlus : ModelData::AxisDirection::XMinus;
        break;
    case 1:
        value = sign > 0 ? ModelData::AxisDirection::YPlus : ModelData::AxisDirection::YMinus;
        break;
    case 2:
        value = sign > 0 ? ModelData::AxisDirection::ZPlus : ModelData::AxisDirection::ZMinus;
        break;
    }
}

}
