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

using namespace std;


/// This class can load a model, and then apply a texture on it to finally render it
class LIB3DS_QTSHARED_EXPORT Model
{
public:
    Model();
    ~Model(); /// RAII -> free file, free textures

    /// It loads the file 'name', sets the current frame to 0 and if the model has textures, it will be applied to the model
    void loadFile(const char *name);
    /// Creates a display list that sets all the information for the lights.
    void CreateLightList();
    /// It renders the node specified by the argument and sets material properties to the node if nescesary
    void renderNode(Lib3dsNode *node);
    /// Enables all lights in the model if not already enabled
    void EnableLights();
    /// Disables all lights in the model if not already disabled
    void DisableLights();
    /// It renders the model, by rendering node by node using the renderNode function.But before it's renderd it's translated to (x,y,z) and then rotates it angle degrees

    void RenderModel();
    /// It applies a texture to mesh ,according to the data that mesh contains
    void ApplyTexture(Lib3dsMesh *mesh);
    Lib3dsFile * get3DSPointer();
    string getFilename();
private:
    Lib3dsFile *file; /**< file holds the data of the model */
    const char *filename; /**< It's the filename of the model */
    int curFrame; /**< curFrame keeps track of the current frame that shuold be renderd */
    vector<GLuint> textureIndices; /**< this variable holds the texture indicies */
    vector<string> textureFilenames; /**< Holds the filenames of the textures, so I can see wheter a texture is used multiple times or not */
    bool lightEnabled; /**< wheter light was enabled before this class. */
    GLuint lightListIndex;
};

#endif // MODEL_H
