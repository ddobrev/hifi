//
//  BlendFace.cpp
//  interface
//
//  Created by Andrzej Kapolka on 9/16/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#include <QNetworkReply>

#include "Application.h"
#include "BlendFace.h"
#include "Head.h"

using namespace fs;
using namespace std;

BlendFace::BlendFace(Head* owningHead) :
    _owningHead(owningHead)
{
    // we may have been created in the network thread, but we live in the main thread
    moveToThread(Application::getInstance()->thread());
}

BlendFace::~BlendFace() {
    deleteGeometry();
}

ProgramObject BlendFace::_eyeProgram;

void BlendFace::init() {
    if (!_eyeProgram.isLinked()) {
        switchToResourcesParentIfRequired();
        _eyeProgram.addShaderFromSourceFile(QGLShader::Vertex, "resources/shaders/eye.vert");
        _eyeProgram.addShaderFromSourceFile(QGLShader::Fragment, "resources/shaders/iris.frag");
        _eyeProgram.link();
        
        _eyeProgram.bind();
        _eyeProgram.setUniformValue("texture", 0);
        _eyeProgram.release();
    }
}

const glm::vec3 MODEL_TRANSLATION(0.0f, -120.0f, 40.0f); // temporary fudge factor
const float MODEL_SCALE = 0.0006f;

bool BlendFace::render(float alpha) {
    if (!isActive()) {
        return false;
    }

    // set up blended buffer ids on first render after load
    const FBXGeometry& geometry = _geometry->getFBXGeometry();
    const QVector<NetworkMesh>& networkMeshes = _geometry->getMeshes();
    if (_blendedVertexBufferIDs.isEmpty()) {
        foreach (const FBXMesh& mesh, geometry.meshes) {
            GLuint id = 0;
            if (!mesh.blendshapes.isEmpty()) {
                glGenBuffers(1, &id);
                glBindBuffer(GL_ARRAY_BUFFER, id);
                glBufferData(GL_ARRAY_BUFFER, (mesh.vertices.size() + mesh.normals.size()) * sizeof(glm::vec3),
                    NULL, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            _blendedVertexBufferIDs.append(id);
        }
        
        // make sure we have the right number of dilated texture pointers
        _dilatedTextures.resize(geometry.meshes.size());
    }

    glPushMatrix();
    glTranslatef(_owningHead->getPosition().x, _owningHead->getPosition().y, _owningHead->getPosition().z);
    glm::quat orientation = _owningHead->getOrientation();
    glm::vec3 axis = glm::axis(orientation);
    glRotatef(glm::angle(orientation), axis.x, axis.y, axis.z);    
    glm::vec3 scale(-_owningHead->getScale() * MODEL_SCALE, _owningHead->getScale() * MODEL_SCALE,
        -_owningHead->getScale() * MODEL_SCALE);
    glScalef(scale.x, scale.y, scale.z);

    glm::vec3 offset = MODEL_TRANSLATION - geometry.neckPivot;
    glTranslatef(offset.x, offset.y, offset.z);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // enable normalization under the expectation that the GPU can do it faster
    glEnable(GL_NORMALIZE); 
    glEnable(GL_TEXTURE_2D);
    
    glColor4f(_owningHead->getSkinColor().r, _owningHead->getSkinColor().g, _owningHead->getSkinColor().b, alpha);
    
    for (int i = 0; i < networkMeshes.size(); i++) {
        const NetworkMesh& networkMesh = networkMeshes.at(i);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, networkMesh.indexBufferID);
        
        const FBXMesh& mesh = geometry.meshes.at(i);    
        int vertexCount = mesh.vertices.size();
        
        glPushMatrix();
        
        // apply eye rotation if appropriate
        Texture* texture = networkMesh.diffuseTexture.data();
        if (mesh.isEye) {
            glTranslatef(mesh.pivot.x, mesh.pivot.y, mesh.pivot.z);
            glm::quat rotation = glm::inverse(orientation) * _owningHead->getEyeRotation(orientation *
                (mesh.pivot * scale + MODEL_TRANSLATION) + _owningHead->getPosition());
            glm::vec3 rotationAxis = glm::axis(rotation);
            glRotatef(glm::angle(rotation), -rotationAxis.x, rotationAxis.y, -rotationAxis.z);
            glTranslatef(-mesh.pivot.x, -mesh.pivot.y, -mesh.pivot.z);
        
            _eyeProgram.bind();
            
            if (texture != NULL) {
                texture = (_dilatedTextures[i] = static_cast<DilatableNetworkTexture*>(texture)->getDilatedTexture(
                    _owningHead->getPupilDilation())).data();
            }
        }
        
        glMultMatrixf((const GLfloat*)&mesh.transform);
        
        // all meshes after the first are white
        if (i == 1) {
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
        }
        
        glBindTexture(GL_TEXTURE_2D, texture == NULL ? 0 : texture->getID());
        
        glBindBuffer(GL_ARRAY_BUFFER, networkMesh.vertexBufferID);
        if (mesh.blendshapes.isEmpty()) {
            glTexCoordPointer(2, GL_FLOAT, 0, (void*)(vertexCount * 2 * sizeof(glm::vec3)));    
        
        } else {
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
        
            _blendedVertices.resize(max(_blendedVertices.size(), vertexCount));
            _blendedNormals.resize(_blendedVertices.size());
            memcpy(_blendedVertices.data(), mesh.vertices.constData(), vertexCount * sizeof(glm::vec3));
            memcpy(_blendedNormals.data(), mesh.normals.constData(), vertexCount * sizeof(glm::vec3));
            
            // blend in each coefficient
            const vector<float>& coefficients = _owningHead->getBlendshapeCoefficients();
            for (int j = 0; j < coefficients.size(); j++) {
                float coefficient = coefficients[j];
                if (coefficient == 0.0f || j >= mesh.blendshapes.size() || mesh.blendshapes[j].vertices.isEmpty()) {
                    continue;
                }
                const float NORMAL_COEFFICIENT_SCALE = 0.01f;
                float normalCoefficient = coefficient * NORMAL_COEFFICIENT_SCALE;
                const glm::vec3* vertex = mesh.blendshapes[j].vertices.constData();
                const glm::vec3* normal = mesh.blendshapes[j].normals.constData();
                for (const int* index = mesh.blendshapes[j].indices.constData(),
                        *end = index + mesh.blendshapes[j].indices.size(); index != end; index++, vertex++, normal++) {
                    _blendedVertices[*index] += *vertex * coefficient;
                    _blendedNormals[*index] += *normal * normalCoefficient;
                }
            }
    
            glBindBuffer(GL_ARRAY_BUFFER, _blendedVertexBufferIDs.at(i));
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(glm::vec3), _blendedVertices.constData());
            glBufferSubData(GL_ARRAY_BUFFER, vertexCount * sizeof(glm::vec3),
                vertexCount * sizeof(glm::vec3), _blendedNormals.constData());
        }
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glNormalPointer(GL_FLOAT, 0, (void*)(vertexCount * sizeof(glm::vec3)));
        
        glDrawRangeElementsEXT(GL_QUADS, 0, vertexCount - 1, mesh.quadIndices.size(), GL_UNSIGNED_INT, 0);
        glDrawRangeElementsEXT(GL_TRIANGLES, 0, vertexCount - 1, mesh.triangleIndices.size(),
            GL_UNSIGNED_INT, (void*)(mesh.quadIndices.size() * sizeof(int)));
            
        if (mesh.isEye) {
            _eyeProgram.release();
        }
        
        glPopMatrix();
    }
    
    glDisable(GL_NORMALIZE); 
    glDisable(GL_TEXTURE_2D);
    
    // deactivate vertex arrays after drawing
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // bind with 0 to switch back to normal operation
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glPopMatrix();

    return true;
}

void BlendFace::getEyePositions(glm::vec3& firstEyePosition, glm::vec3& secondEyePosition) const {
    if (!isActive()) {
        return;
    }
    
    glm::quat orientation = _owningHead->getOrientation();
    glm::vec3 scale(-_owningHead->getScale() * MODEL_SCALE, _owningHead->getScale() * MODEL_SCALE,
        -_owningHead->getScale() * MODEL_SCALE);
    bool foundFirst = false;
    
    const FBXGeometry& geometry = _geometry->getFBXGeometry();
    foreach (const FBXMesh& mesh, geometry.meshes) {
        if (mesh.isEye) {
            glm::vec3 position = orientation * ((mesh.pivot + MODEL_TRANSLATION - geometry.neckPivot) * scale) +
                _owningHead->getPosition();
            if (foundFirst) {
                secondEyePosition = position;
                return;
            }
            firstEyePosition = position;
            foundFirst = true;
        }
    }
}

void BlendFace::setModelURL(const QUrl& url) {
    // don't recreate the geometry if it's the same URL
    if (_modelURL == url) {
        return;
    }
    _modelURL = url;

    // delete our local geometry and custom textures
    deleteGeometry();
    _dilatedTextures.clear();
    
    _geometry = Application::getInstance()->getGeometryCache()->getGeometry(url);
}

void BlendFace::deleteGeometry() {
    foreach (GLuint id, _blendedVertexBufferIDs) {
        glDeleteBuffers(1, &id);
    }
    _blendedVertexBufferIDs.clear();
}