//
//  VoxelFade.h
//  interface
//
//  Created by Brad Hefta-Gaub on 8/6/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#ifndef __interface__VoxelFade__
#define __interface__VoxelFade__

#include <OctalCode.h> // for VoxelPositionSize

class VoxelFade {
public:

    enum FadeDirection { FADE_OUT, FADE_IN};
    static const float FADE_OUT_START;
    static const float FADE_OUT_END;
    static const float FADE_OUT_STEP;
    static const float FADE_IN_START;
    static const float FADE_IN_END;
    static const float FADE_IN_STEP;
    static const float DEFAULT_RED;
    static const float DEFAULT_GREEN;
    static const float DEFAULT_BLUE;

    VoxelPositionSize   voxelDetails;
    FadeDirection       direction;
    float               opacity;
    
    float               red;
    float               green;
    float               blue;
    
    VoxelFade(FadeDirection direction = FADE_OUT, float red = DEFAULT_RED, 
              float green = DEFAULT_GREEN, float blue = DEFAULT_BLUE);
    
    void render();
    bool isDone() const;
};

#endif // __interface__VoxelFade__
