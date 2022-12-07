#include "model.h"

namespace ofbxqt
{

Model::Model(std::shared_ptr<ModelData> data_)
    : armature(data_->armature)
    , data(data_)
{
    if (!data)
    {
        qCritical() << Q_FUNC_INFO << "data is null";
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

    if (!data)
    {
        qCritical() << Q_FUNC_INFO << "data is null";
        return;
    }

    if (!material)
    {
        material = data->material;
    }

    if (material)
    {
        material->initializeGL();
    }

    if (!data->vertexBuffer.isCreated())
    {
        if (!data->vertexBuffer.create())
        {
            qCritical() << Q_FUNC_INFO << "failed to create vertex buffer";
        }

        if (!data->vertexBuffer.bind())
        {
            qCritical() << Q_FUNC_INFO << "failed to bind vertex buffer";
        }

        if (data->vertexData.isEmpty())
        {
            qCritical() << Q_FUNC_INFO << "vertex data is empty";
        }

        if (data->vertexCount <= 0)
        {
            qCritical() << Q_FUNC_INFO << "vertex count is" << data->vertexCount;
        }

        if (data->vertexStride <= 0)
        {
            qCritical() << Q_FUNC_INFO << "vertex stride is" << data->vertexStride;
        }

        data->vertexBuffer.allocate(data->vertexData, data->vertexCount * data->vertexStride);

        if (data->vertexBuffer.size() <= 0)
        {
            qWarning() << Q_FUNC_INFO << "vertex buffer is empty";
        }

        data->vertexBuffer.release();

        data->vertexData.clear();
    }

    if (!data->indexBuffer.isCreated())
    {
        if (!data->indexBuffer.create())
        {
            qCritical() << Q_FUNC_INFO << "failed to create index buffer";
        }

        if (!data->indexBuffer.bind())
        {
            qCritical() << Q_FUNC_INFO << "failed to bind index buffer";
        }

        if (data->indexData.isEmpty())
        {
            qCritical() << Q_FUNC_INFO << "index data is empty";
        }

        if (data->indexCount <= 0)
        {
            qCritical() << Q_FUNC_INFO << "index count is" << data->indexCount;
        }

        if (data->indexStride <= 0)
        {
            qCritical() << Q_FUNC_INFO << "index stride is" << data->indexStride;
        }

        data->indexBuffer.allocate(data->indexData, data->indexCount * data->indexStride);

        if (data->indexBuffer.size() <= 0)
        {
            qWarning() << Q_FUNC_INFO << "index buffer is empty";
        }

        data->indexBuffer.release();

        data->indexData.clear();
    }

    if (!data->shader.isLinked())
    {
        QString vshaderFileName;
        if (data->armature.getJoints().count() > 0)
        {
            vshaderFileName = ":/OpenFBXQt-shaders/vshader-with-joins.glsl";
        }
        else
        {
            vshaderFileName = ":/OpenFBXQt-shaders/vshader-no-joints.glsl";
        }

        QString fshaderFileName;
        if (material->diffuseTexture)
        {
            fshaderFileName = ":/OpenFBXQt-shaders/fshader-textured.glsl";
        }
        else
        {
            fshaderFileName = ":/OpenFBXQt-shaders/fshader-colored.glsl";
        }

        if (!data->shader.addShaderFromSourceFile(QOpenGLShader::Vertex, vshaderFileName))
        {
            qWarning() << Q_FUNC_INFO << "failed to compile vertex shader";
        }

        if (!data->shader.addShaderFromSourceFile(QOpenGLShader::Fragment, fshaderFileName))
        {
            qWarning() << Q_FUNC_INFO << "failed to compile fragment shader";
        }

        if (!data->shader.link())
        {
            qWarning() << Q_FUNC_INFO << "failed to link shader";
        }
    }
}

void Model::paintGL(const QMatrix4x4 &projection)
{
    if (!data)
    {
        qCritical() << Q_FUNC_INFO << "data is null";
        return;
    }

    if (!data->shader.isLinked())
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
    }

    if (material->diffuseTexture)
    {
        if (material->diffuseTexture->texture)
        {
            material->diffuseTexture->texture->bind();
        }
        else
        {
#ifdef QT_DEBUG
            qCritical() << Q_FUNC_INFO << "diffuse texture is null";
#endif
        }
    }

    if (!data->shader.bind())
    {
#ifdef QT_DEBUG
        qCritical() << Q_FUNC_INFO << "failed to bind shader";
#endif
    }

    QVector3D v(0, 0, 0);
    v = v.unproject(matrix * data->sourceMatrix, projection, QRect(0, 0, 1, 1));

    data->shader.setUniformValue("projection_pos", v);
    data->shader.setUniformValue("model_projection_matrix", projection * matrix * data->sourceMatrix);

    if (material)
    {
        if (material->diffuseTexture && material->diffuseTexture->texture)
        {
            data->shader.setUniformValue("texture", 0);
        }
        else if (material->diffuseColor)
        {
            data->shader.setUniformValue("u_color", *material->diffuseColor);
        }
        else
        {
#ifdef QT_DEBUG
        qCritical() << Q_FUNC_INFO << "diffuse color is null";
#endif
        data->shader.setUniformValue("u_color", QColor());
        }
    }
    else
    {
        data->shader.setUniformValue("u_color", QColor());
    }

    if (needUpdateArmature)
    {
        armature.update();
        needUpdateArmature = false;
    }

    const QVector<QMatrix4x4>& matrices = armature.jointsResultMatrices;
    if (matrices.count() > 0)
    {
        data->shader.setUniformValueArray("joints", matrices.data(), matrices.count());
    }

    data->vertexBuffer.bind();
    data->indexBuffer.bind();

    for (const VertexAttributeInfo& attribute : qAsConst(data->vertexAttributes))
    {
        const int location = data->shader.attributeLocation(attribute.nameForShader);
        if (location == -1)
        {
#ifdef QT_DEBUG
            qWarning() << Q_FUNC_INFO << "name" << attribute.nameForShader << "not valid for this shader";
#endif
            continue;
        }

        data->shader.enableAttributeArray(location);
        data->shader.setAttributeBuffer(location, attribute.type, attribute.offset, attribute.tupleSize, data->vertexStride);
    }

    glDrawElements(data->drawElementsMode, data->indexCount, data->indexType, nullptr);

    data->shader.release();

    if (material && material->diffuseTexture && material->diffuseTexture->texture)
    {
        material->diffuseTexture->texture->release();
    }

    data->vertexBuffer.release();
    data->indexBuffer.release();
}

}
