//
//  VoxelShader.h
//  interface
//
//  Created by Brad Hefta-Gaub on 9/23/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#ifndef __interface__VoxelShader__
#define __interface__VoxelShader__

#include <QObject>

class ProgramObject;

/// A generic full screen glow effect.
class VoxelShader : public QObject {
    Q_OBJECT
    
public:
    VoxelShader();
    ~VoxelShader();
    
    void init();
    
    /// Starts using the voxel geometry shader effect.
    void begin();
    
    /// Stops using the voxel geometry shader effect.
    void end();
    
    /// Gets access to attributes from the shader program
    int attributeLocation(const char * name) const;

    static ProgramObject* createGeometryShaderProgram(const QString& name);
    
public slots:
    
private:
    
    bool _initialized;

    ProgramObject* _program;
};

#endif /* defined(__interface__VoxelShader__) */
