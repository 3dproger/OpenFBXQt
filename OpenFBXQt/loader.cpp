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

Loader::Loader()
{

}

QList<Model*> Loader::open(const QString &fileName, const OpenModelConfig config_, QList<Note>* notes_)
{
    config = config_;
    notes = notes_;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        addNote(Note::Type::Error, QTranslator::tr("Failed to open file \"%1\", error: \"%2\"").arg(fileName, file.errorString()));
        return QList<Model*>();
    }

    const QByteArray rawData = file.readAll();
    file.close();

    ofbx::IScene* scene = ofbx::load((ofbx::u8*)rawData.constData(), rawData.size(), (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
    if (!scene)
    {
        addNote(Note::Type::Error, QTranslator::tr("No scene"));
        return QList<Model*>();
    }

    const int meshCount = scene->getMeshCount();
    if (meshCount <= 0)
    {
        scene->destroy();
        addNote(Note::Type::Error, QTranslator::tr("No meshes in scene"));
        return QList<Model*>();
    }

    const QFileInfo fileInfo(fileName);
    const QString absoluteDirectoryPath = fileInfo.absoluteDir().absolutePath();

    QList<Model*> models;
    for (int i = 0; i < meshCount; ++i)
    {
        Model* model = loadMesh(scene->getMesh(i), i, absoluteDirectoryPath);
        if (model)
        {
            models.append(model);
        }
    }

    scene->destroy();

    return models;
}

void Loader::addNote(const Note::Type type, const QString &text)
{
    if (notes)
    {
        notes->append(Note(type, text));
    }
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
            addNote(Note::Type::Warning, QTranslator::tr("Internal error"));
            qWarning() << Q_FUNC_INFO << "cluster is null at index" << clusterNum;
            continue;
        }

        const ofbx::Object* object = cluster->getLink();
        if (!object)
        {
            addNote(Note::Type::Warning, QTranslator::tr("Internal error"));
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
            const QString newName = name + "_1";
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
            addNote(Note::Type::Warning, QTranslator::tr("No cluster for joint \"%1\"").arg(joint->getName()));
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
            addNote(Note::Type::Warning, QTranslator::tr("Joint indices count and joint weights count do not match for joint \"%1\"").arg(joint->getName()));
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

Model *Loader::loadMesh(const ofbx::Mesh *mesh, const int meshIndex, const QString& absoluteDirectoryPath)
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

    Material* material = nullptr;

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
            material = loadMaterial(mesh->getMaterial(0), meshIndex, 0, absoluteDirectoryPath);
        }
        else
        {
            addNote(Note::Type::Warning, QTranslator::tr("No materials. Mesh %1").arg(meshIndex));
            qWarning() << Q_FUNC_INFO << "no materials. Mesh" << meshIndex;
        }
    }

    ModelData* data = new ModelData();

    data->material = material;
    data->sourceMatrix = convertMatrix4x4(mesh->getLocalTransform());

    QHash<GLuint, QVector<QPair<GLuint, GLfloat>>> jointsData;

    const ofbx::Skin* skin = geometry->getSkin();
    if (skin)
    {
        loadJoints(skin, *data, jointsData);
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

    static const int MaxJointsForVertex = 4;
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
                if (jointCountForVertex > MaxJointsForVertex)
                {
                    foundTooMuchJoints = true;
                }

                for (int jointNum = 0; jointNum < qMin(jointCountForVertex, MaxJointsForVertex); ++jointNum)
                {
                    rawVertexArray[idx++] = (GLfloat)jointsData[vertexIndex][jointNum].second;
                }

                for (int jointNum = jointCountForVertex; jointNum < MaxJointsForVertex; ++jointNum)
                {
                    rawVertexArray[idx++] = (GLfloat)0.0;
                }

                for (int jointNum = 0; jointNum < qMin(jointCountForVertex, MaxJointsForVertex); ++jointNum)
                {
                    rawVertexArray[idx++] = (GLfloat)jointsData[vertexIndex][jointNum].first;
                }

                for (int jointNum = jointCountForVertex; jointNum < MaxJointsForVertex; ++jointNum)
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
        addNote(Note::Type::Warning, QTranslator::tr("More than %1 joint weights per vertex not supported. Extra weights will be ignored. Mesh %2")
                          .arg(MaxJointsForVertex).arg(meshIndex));
        qWarning() << Q_FUNC_INFO << "more than" << MaxJointsForVertex << "joint weights per vertex not supported. Extra weights will be ignored, mesh" << meshIndex;
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

    data->skeleton.update();

    ModelDataStorage::data.append(data);

    return new Model(*data);
}

Material *Loader::loadMaterial(const ofbx::Material *rawMaterial, const int meshIndex, const int materialIndex, const QString& absoluteDirectoryPath)
{
    if (!rawMaterial)
    {
        addNote(Note::Type::Error, QTranslator::tr("Internal error"));
        qCritical() << Q_FUNC_INFO << "raw material is null, mesh index =" << meshIndex << ", material index =" << materialIndex;
        return nullptr;
    }

    Material* material = nullptr;

    if (config.loadMaterialTexture)
    {
        for (int i = 0; i < (int)ofbx::Texture::TextureType::COUNT; ++i)
        {
            const ofbx::Texture::TextureType type = (ofbx::Texture::TextureType)i;
            const QString textureTypeStr = textureTypeToString(type);

            const ofbx::Texture* texture = rawMaterial->getTexture(type);
            if (texture)
            {
                bool isSupportedTextureType = false;
                switch (type)
                {
                case ofbx::Texture::DIFFUSE:
                {
                    isSupportedTextureType = true;

                    QImage image;
                    QString fileName;
                    if (loadImage(image, fileName, texture, absoluteDirectoryPath, meshIndex, materialIndex, type))
                    {
                        material = new TextureMaterial(image, fileName);
                    }
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
    }

    if (!material)
    {
        if (config.loadMaterialColor)
        {
            material = new ColorMaterial(covertColor(rawMaterial->getDiffuseColor()));
        }
    }

    return material;
}

bool Loader::loadImage(QImage &image, QString& resultFileName, const ofbx::Texture* texture, const QString &absoluteDirectoryPath, const int meshIndex, const int materialIndex, ofbx::Texture::TextureType type)
{
    image = QImage();
    resultFileName = QString();

    const QString textureTypeStr = textureTypeToString(type);

    if (!texture)
    {
        addNote(Note::Type::Error, QTranslator::tr("Internal error"));
        qCritical() << Q_FUNC_INFO << "texture is null. Mesh " << meshIndex << ", material" << materialIndex << ", texture" << textureTypeStr;
        return false;
    }

    //TODO: implement search in relative directory path
    const QString rawRelativeFileName = convertString2048(texture->getRelativeFileName());
    const QString relativeFileName = QFileInfo(rawRelativeFileName).fileName();

    if (relativeFileName.isEmpty())
    {
        addNote(Note::Type::Warning, QTranslator::tr("Empty texture image relative file name. Mesh %1, material %2, texture %3")
                          .arg(meshIndex).arg(materialIndex).arg(textureTypeStr));
        qWarning() << Q_FUNC_INFO << "empty texture image relative file name. Mesh " << meshIndex << ", material" << materialIndex << ", texture" << textureTypeStr;
    }
    else
    {
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
        }
        else if (!fileInfo.isReadable())
        {
            addNote(Note::Type::Error, QTranslator::tr("File \"%1\" not readable. Mesh %2, material %3, texture %4")
                              .arg(fileName).arg(meshIndex).arg(materialIndex).arg(textureTypeStr));
            qCritical() << Q_FUNC_INFO << "file" << fileName << "not readable. Mesh " << meshIndex << ", material" << materialIndex << ", texture" << textureTypeStr;
        }
        else
        {
            if (image.load(fileName))
            {
                resultFileName = fileName;
                addNote(Note::Type::Info, QTranslator::tr("Opened image \"%1\". Mesh %2, material %3, texture %4")
                                  .arg(fileName).arg(meshIndex).arg(materialIndex).arg(textureTypeStr));

                return true;
            }
            else
            {
                addNote(Note::Type::Error, QTranslator::tr("Failed to open image \"%1\". Mesh %2, material %3, texture %4")
                                  .arg(fileName).arg(meshIndex).arg(materialIndex).arg(textureTypeStr));
                qCritical() << Q_FUNC_INFO << "failed to open image" << fileName << ". Mesh " << meshIndex << ", material" << materialIndex << ", texture" << textureTypeStr;
            }
        }
    }

    return false;
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

}
