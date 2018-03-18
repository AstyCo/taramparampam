/*
Program name:Combat of Death
Description source file: this class loads 3ds models
    Copyright (C) 2005  Hylke Donker

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

/**
\file 3DS.cpp
\brief The cpp file of 3DS.h

This is the place were all the functions are 'defined'
*/

#include "model.h"

#include <QImage>
#include <QGLWidget>
#include <qmath.h>

#include <QDebug>

// constructor, enables and set properties of texture coordinate generation and set the current frame
Model::Model()
{
    curFrame = 0; // current frame of our model
    if(glIsEnabled(GL_LIGHTING))
        lightEnabled = true;
    else
        lightEnabled = false;
}

// destructor, free up memory and disable texture generation
Model::~Model()
{
    if(file) // if the file isn't freed yet
        lib3ds_file_free(file); //free up memory
     //disable texture generation
    for(unsigned int i = 0;i < textureIndices.size();i++)
        glDeleteTextures(1, &textureIndices.at(i));
}

// load the model, and if the texture has textures, then apply them on the geometric primitives
void Model::loadFile(const char *name)
{
    filename = name;
    // load file
    file = lib3ds_file_load(filename);
    if(!file) // if we were not able to load the file
    {
        // give some errors
        QString event = "Error loading: ";
        QString online = "On line 61 in file ";
        online.append(__FILE__);
        event.append(filename);
        qDebug() << event << endl;
        qDebug() << online << endl;
        Q_ASSERT(false);
    }
    lib3ds_file_eval(file, 0); // set current frame to 0
    // apply texture to all meshes that have texels
    Lib3dsMesh *mesh;
    for(mesh = file->meshes;mesh != 0;mesh = mesh->next)
    {
        if(mesh->texels) //if there's texels for the mesh
            ApplyTexture(mesh); //then apply texture to it
    }
    if(file->lights) //if we have lights in our model
        CreateLightList();
}

void Model::CreateLightList()
{
    lightListIndex = glGenLists(1);
    glNewList(lightListIndex, GL_COMPILE_AND_EXECUTE);
    Lib3dsLight *light; // temporary variable to store our lights
    GLint curlight = GL_LIGHT0;
    for(light = file->lights;light != 0;light = light->next)
    {
        if(light->off) //if our light is off
            continue; //Move to the next light

        GLfloat diff[4], amb[4], pos[4];

        for(int j = 0;j < 3;j++)
        {
            diff[j] = light->color[j];
            amb[j] = light->color[j] / 4.5; // We might wanna change this;
            pos[j] = light->position[j];
        }

        diff[3] = amb[3] = pos[3] = 1.0;

        curlight++;
        // Start setting light
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diff); //set the diffuse color
        glLightfv(GL_LIGHT0, GL_POSITION, pos); //set the position of the light
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb); // set the ambient color of the light
        glLightfv(GL_LIGHT0, GL_SPECULAR, diff); // set the specular color of the light

        if(light->spot) // if it's a light spot
        {
            for(int i = 0;i < 3;i++) // get position of the light
                pos[i] = light->spot[i] - light->position[i];
            pos[3] = light->spot[3] - light->position[3];
            glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, pos); //specify that we have a spot at position 'pos'
        }
    }
    glEndList();
}

// what is basicly does is, set the properties of the texture for our mesh
void Model::ApplyTexture(Lib3dsMesh *mesh)
{
    for(unsigned int i = 0;i < mesh->faces;i++)
    {
        Lib3dsFace *f = &mesh->faceL[i];
        if(! f->material[0])
            continue;
        QImage img;
        Lib3dsMaterial *mat;
        bool found = false;
        mat = lib3ds_file_material_by_name(file, f->material);
        for(unsigned int i = 0;i < textureFilenames.size();i++)
        {
            if(strcmp(mat->texture1_map.name, textureFilenames.at(i).c_str()) == 0)
            {
                textureIndices.push_back(textureIndices.at(i));
                textureFilenames.push_back(mat->texture1_map.name);
                found = true;
                break;
            }
        }
        if(!found)
        {
            textureFilenames.push_back(mat->texture1_map.name);
            if(!img.load(mat->texture1_map.name))
            {
                Q_ASSERT_X(false,"Model::ApplyTexture" , "Error loading 3ds texture, perhaps file format not supported?");
                return;
            }
            img = QGLWidget::convertToGLFormat(img);
            /// TODO my load texture
            GLuint tmpIndex; // temporary index to old the place of our texture
            glGenTextures(1, &tmpIndex); // allocate memory for one texture
            textureIndices.push_back(tmpIndex); // add the index of our newly created texture to textureIndices
            glBindTexture(GL_TEXTURE_2D, tmpIndex); // use our newest texture
//            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, img.width() , img.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits()); // genereate MipMap levels for our texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // give the best result for texture magnification
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //give the best result for texture minification
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // don't repeat texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // don't repeat texture
        }
    }
}

// this code is rewritten from player.c example that came with lib3ds
// what it does is render a node from our model
void Model::renderNode(Lib3dsNode *node)
{
    Q_ASSERT(file); //this is for debugging
    {
        Lib3dsNode *tmp;
        for(tmp = node->childs;tmp != 0;tmp = tmp->next)
            renderNode(tmp); //render all child nodes of this note
    }
    if(node->type == LIB3DS_OBJECT_NODE) //check wheter the node is a 3ds node
    {
        if(! node->user.d) //Wheter we have a list or not, if not we're gonna create one
        {
            Lib3dsMesh *mesh = lib3ds_file_mesh_by_name(file, node->name); //get all the meshes of the current node
            Q_ASSERT(mesh); //for debugging in case we don't have a mesh
            if(! mesh)
                return;
            GL_CHECK( node->user.d = glGenLists(1);) //alocate memory for one list
            GL_CHECK( glNewList(node->user.d, GL_COMPILE);) //here we create our list
            {
                unsigned p;
                Lib3dsVector *normals;
                normals = static_cast<float(*)[3]> (std::malloc (3*sizeof(Lib3dsVector)*mesh->faces)); //alocate memory for our normals
                {
                    Lib3dsMatrix m;
                    lib3ds_matrix_copy(m, mesh->matrix); //copy the matrix of the mesh in our temporary matrix
                    lib3ds_matrix_inv(m);
                    glMultMatrixf(&m[0][0]); //adjust our current matrix to the matrix of the mesh
                }
                lib3ds_mesh_calculate_normals(mesh, normals); //calculate the normals of the mesh
                int j = 0;
                for(p = 0;p < mesh->faces;p++)
                {
                    Lib3dsFace *f = &mesh->faceL[p];
                    Lib3dsMaterial *mat=0;
                    if(f->material[0]) //if the face of the mesh has material properties
                        mat = lib3ds_file_material_by_name(file, f->material); //read material properties from file
                    if(mat) //if we have material
                    {
                        static GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
                        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient); // Ambient color
                        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse); //diffuse color
                        glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular); //specular color
                        float shine;
                        shine = pow(2, 10.0 * mat->shininess);
                        if(shine > 128.0)
                            shine = 128.0;
                        glMaterialf(GL_FRONT, GL_SHININESS, shine);
                    }
                    else // if we do not have material properties, we have to set them manually
                    {
                        GLfloat diff[4] = { 0.75, 0.75, 0.75, 1.0 }; // color: white/grey
                        GLfloat amb[4] = { 0.25, 0.25, 0.25, 1.0 }; //color: black/dark gray
                        GLfloat spec[4] = { 0.0, 0.0, 0.0, 1.0 }; //color: completly black
                        glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
                        glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
                        glMaterialfv(GL_FRONT, GL_AMBIENT, spec);
                    }
                    {
                        if(mesh->texels)
                        {
                            glBindTexture(GL_TEXTURE_2D, textureIndices.at(j));
                            j++;
                        }
                        glBegin(GL_TRIANGLES);
                        for(int i = 0;i < 3;i++)
                        {
                            glNormal3fv(normals[3*p+i]); //set normal vector of that point
                            if(mesh->texels)
                                glTexCoord2f(mesh->texelL[f->points[i]][0], mesh->texelL[f->points[i]][1]);
                            glVertex3fv(mesh->pointL[f->points[i]].pos); //Draw the damn triangle
                        }
                        glEnd();
                    }
                }
                free(normals); //free up memory
            }
            glEndList(); // end of list
        }
        if(node->user.d) // if we have created a link list(with glNewList)
        {
            Lib3dsObjectData *tmpdat;
            glPushMatrix(); //save transformation values
            tmpdat = &node->data.object; // get the position data
            glMultMatrixf(&node->matrix[0][0]); //adjust matrix according to the node
            glTranslatef(-tmpdat->pivot[0], -tmpdat->pivot[1], -tmpdat->pivot[2]); //move to the right place;
            glCallList(node->user.d); //render node
            glPopMatrix(); //return transformation original values
        }
    }
}

// this function actually renders the model at place (x, y, z) and then rotated around the y axis by 'angle' degrees
void Model::RenderModel()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);

    if(file->lights) //if we have lights in the model
    {
        EnableLights(); //enable all lights
        glCallList(lightListIndex); //set lights.
    }

    Lib3dsNode *nodes;
    for(nodes = file->nodes;nodes != 0;nodes = nodes->next) // Render all nodes
        renderNode(nodes);


    if(file->lights)
        DisableLights(); // disable lighting, we don't want it have it enabled longer than necessary

    curFrame++;
    if(curFrame > file->frames) //if the next frame doesn't exist, go to frame 0
        curFrame = 0;
    lib3ds_file_eval(file, curFrame); // set current frame

    glDisable(GL_CULL_FACE);
    glShadeModel(GL_FLAT);
}

void Model::EnableLights()
{
    glEnable(GL_LIGHTING);
    GLuint lightNum = GL_LIGHT0;
    Lib3dsLight *light;
    for(light = file->lights;light != 0;light = light->next)
    {
        if(!glIsEnabled(lightNum))
            glEnable(lightNum);
        lightNum++;
    }
}

void Model::DisableLights()
{
    glDisable(GL_LIGHTING);
    GLuint lightNum = GL_LIGHT0;
    Lib3dsLight *light;
    for(light = file->lights;light != 0;light = light->next)
    {
        if(glIsEnabled(lightNum))
            glDisable(lightNum);
        lightNum++;
    }
}

Lib3dsFile * Model::get3DSPointer()
{
    return file;
}

string Model::getFilename()
{
    string returnvalue = filename;
    return returnvalue;
}
