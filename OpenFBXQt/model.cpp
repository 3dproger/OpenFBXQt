#include "model.h"

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

Model::Model(ModelData& data_)
    : skeleton(data_.skeleton)
    , data(data_)
{
    if (!shuffledSpareColor)
    {
        shuffledSpareColor = true;

        std::random_shuffle(spareColors.begin(), spareColors.end());
    }
}

Model::~Model()
{
    if (material)
    {
        delete material;
        material = nullptr;
    }
}

void Model::initializeGL()
{
    if (initializedGL)
    {
        return;
    }

    initializedGL = true;

    initializeOpenGLFunctions();

    material = findTexture(data.fileName);
    if (!material)
    {
        if (!spareColors.isEmpty())
        {
            if (currentSpareColors >= spareColors.count())
            {
                currentSpareColors = 0;
            }

            ColorMaterial* colorMaterial = new ColorMaterial(spareColors[currentSpareColors]);
            colorMaterial->initializeGL();
            material = colorMaterial;

            currentSpareColors++;
        }
        else
        {
            material = new ColorMaterial(QColor(191, 191, 191));
            qWarning() << Q_FUNC_INFO << "spare colors is empty. Used default material";
        }
    }

    if (!data.vertexBuffer.isCreated())
    {
        if (!data.vertexBuffer.create())
        {
            qCritical() << Q_FUNC_INFO << "failed to create vertex buffer";
        }

        if (!data.vertexBuffer.bind())
        {
            qCritical() << Q_FUNC_INFO << "failed to bind vertex buffer";
        }

        if (data.vertexData.isEmpty())
        {
            qCritical() << Q_FUNC_INFO << "vertex data is empty";
        }

        if (data.vertexCount <= 0)
        {
            qCritical() << Q_FUNC_INFO << "vertex count is" << data.vertexCount;
        }

        if (data.vertexStride <= 0)
        {
            qCritical() << Q_FUNC_INFO << "vertex stride is" << data.vertexStride;
        }

        data.vertexBuffer.allocate(data.vertexData, data.vertexCount * data.vertexStride);

        if (data.vertexBuffer.size() <= 0)
        {
            qWarning() << Q_FUNC_INFO << "vertex buffer is empty";
        }

        data.vertexBuffer.release();

        data.vertexData.clear();
    }

    if (!data.indexBuffer.isCreated())
    {
        if (!data.indexBuffer.create())
        {
            qCritical() << Q_FUNC_INFO << "failed to create index buffer";
        }

        if (!data.indexBuffer.bind())
        {
            qCritical() << Q_FUNC_INFO << "failed to bind index buffer";
        }

        if (data.indexData.isEmpty())
        {
            qCritical() << Q_FUNC_INFO << "index data is empty";
        }

        if (data.indexCount <= 0)
        {
            qCritical() << Q_FUNC_INFO << "index count is" << data.indexCount;
        }

        if (data.indexStride <= 0)
        {
            qCritical() << Q_FUNC_INFO << "index stride is" << data.indexStride;
        }

        data.indexBuffer.allocate(data.indexData, data.indexCount * data.indexStride);

        if (data.indexBuffer.size() <= 0)
        {
            qWarning() << Q_FUNC_INFO << "index buffer is empty";
        }

        data.indexBuffer.release();

        data.indexData.clear();
    }

    if (!data.shader.isLinked())
    {
        QString vshaderFileName;
        if (data.skeleton.getJoints().count() > 0)
        {
            vshaderFileName = ":/OpenFBXQt-shaders/vshader-with-joins.glsl";
        }
        else
        {
            vshaderFileName = ":/OpenFBXQt-shaders/vshader-no-joints.glsl";
        }

        QString fshaderFileName;
        switch (material->type)
        {
        case Material::Type::Image:
            fshaderFileName = ":/OpenFBXQt-shaders/fshader-textured.glsl";
            break;
        case Material::Type::Color:
            fshaderFileName = ":/OpenFBXQt-shaders/fshader-colored.glsl";
            break;
        }

        if (!data.shader.addShaderFromSourceFile(QOpenGLShader::Vertex, vshaderFileName))
        {
            qWarning() << Q_FUNC_INFO << "failed to compile vertex shader";
        }

        if (!data.shader.addShaderFromSourceFile(QOpenGLShader::Fragment, fshaderFileName))
        {
            qWarning() << Q_FUNC_INFO << "failed to compile fragment shader";
        }

        if (!data.shader.link())
        {
            qWarning() << Q_FUNC_INFO << "failed to link shader";
        }
    }
}

void Model::paintGL(const QMatrix4x4 &projection)
{
    if (!data.shader.isLinked())
    {
#ifdef QT_DEBUG
        qCritical() << Q_FUNC_INFO << "shader not linked";
#endif
        return;
    }

    if (!material)
    {
#ifdef QT_DEBUG
        qCritical() << Q_FUNC_INFO << "material is null";
#endif
        return;
    }

    const TextureMaterial* textureMaterial = nullptr;
    const ColorMaterial* colorMaterial = nullptr;

    switch(material->type)
    {
    case ofbxqt::Material::Type::Color:
        colorMaterial = static_cast<const ColorMaterial*>(material);
        break;
    case ofbxqt::Material::Type::Image:
        textureMaterial = static_cast<const TextureMaterial*>(material);
        if (textureMaterial->texture)
        {
            textureMaterial->texture->bind();
        }
        else
        {
#ifdef QT_DEBUG
            qCritical() << Q_FUNC_INFO << "texture is null";
#endif
        }
        break;
    }

    if (!data.shader.bind())
    {
#ifdef QT_DEBUG
        qCritical() << Q_FUNC_INFO << "failed to bind shader";
#endif
    }

    QVector3D v(0, 0, 0);
    v = v.unproject(matrix * data.sourceMatrix, projection, QRect(0, 0, 1, 1));

    data.shader.setUniformValue("projection_pos", v);
    data.shader.setUniformValue("model_projection_matrix", projection * matrix * data.sourceMatrix);
    if (material->type == Material::Type::Image)
    {
        data.shader.setUniformValue("texture", 0);
    }
    else if (material->type == Material::Type::Color)
    {
        if (colorMaterial)
        {
            data.shader.setUniformValue("u_color", colorMaterial->color);
        }
        else
        {
#ifdef QT_DEBUG
            qCritical() << Q_FUNC_INFO << "color material is null";
#endif
        }
    }

    if (needUpdateSkeleton)
    {
        skeleton.update();
        needUpdateSkeleton = false;
    }

    const QVector<QMatrix4x4>& matrices = skeleton.jointsResultMatrices;
    if (matrices.count() > 0)
    {
        data.shader.setUniformValueArray("joints", matrices.data(), matrices.count());
    }

    data.vertexBuffer.bind();
    data.indexBuffer.bind();

    for (const VertexAttributeInfo& attribute : qAsConst(data.vertexAttributes))
    {
        const int location = data.shader.attributeLocation(attribute.nameForShader);
        if (location == -1)
        {
#ifdef QT_DEBUG
            qWarning() << Q_FUNC_INFO << "name" << attribute.nameForShader << "not valid for this shader";
#endif
            continue;
        }

        data.shader.enableAttributeArray(location);
        data.shader.setAttributeBuffer(location, attribute.type, attribute.offset, attribute.tupleSize, data.vertexStride);
    }

    glDrawElements(data.drawElementsMode, data.indexCount, data.indexType, nullptr);

    data.shader.release();

    if (material->type == Material::Type::Image && textureMaterial && textureMaterial->texture)
    {
        textureMaterial->texture->release();
    }

    data.vertexBuffer.release();
    data.indexBuffer.release();
}

void Model::setMaterial(Material *material_)
{
    if (material)
    {
        delete material;
    }

    material = material_;
}

TextureMaterial *Model::findTexture(const QString &fileName)
{
    return nullptr;
}

}
