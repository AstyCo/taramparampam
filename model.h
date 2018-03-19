#ifndef MODEL_H
#define MODEL_H

/**
\file 3DS.h
\brief The 3ds model loader

All models are loaded and renderd using this class
*/

#include "lib3ds_qt_global.h"

#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/matrix.h>
#include <lib3ds/material.h>
#include <lib3ds/light.h>

#include <string>
#include <vector>
#include <GL/gl.h>

#include <QVector>
#include <QMap>
#include <QString>

using namespace std;

namespace lib3ds_qt {

/// This class can load a model, and then apply a texture on it to finally render it
class LIB3DS_QTSHARED_EXPORT Model
{
public:
    Model();
    ~Model(); /// RAII -> free file, free textures

    /// It loads the file 'name', sets the current frame to 0 and if the model has textures, it will be applied to the model
    void loadFile(const char *name);

    void prepareNodes();
    void prepareNode(Lib3dsNode *node);
    int countOfNodesToPrepare() const;
    int countOfNodesToPrepare(Lib3dsNode *node) const;
    void RenderModel();
    /// It applies a texture to mesh ,according to the data that mesh contains
    void ApplyTexture(Lib3dsMesh *mesh);
    Lib3dsFile * get3DSPointer();
    const QString &getFilename() const;

    bool isValid() const;
    bool isValidRadius() const;
    double meshRadius() const;
private:
    Lib3dsFile *_file3ds; /**< file holds the data of the model */
    QString _fileName; /**< It's the filename of the model */
    QMap<QString, GLuint> _textureFilenamesIndexes;
    typedef QMap<QString, GLuint>::iterator MapIterator;

    int _base;
    int _size;
    GLuint _nodeIndex;
    QVector<GLuint> _glListIndexes;
    double _meshRadius;
};

} // namespace lib3ds_qt

#endif // MODEL_H
