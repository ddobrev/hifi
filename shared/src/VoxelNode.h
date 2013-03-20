//
//  VoxelNode.h
//  hifi
//
//  Created by Stephen Birarda on 3/13/13.
//
//

#ifndef __hifi__VoxelNode__
#define __hifi__VoxelNode__

#include <iostream>

class VoxelNode {
public:
    VoxelNode();
    ~VoxelNode();
    
    void addChildAtIndex(int8_t childIndex);
    void setColorFromAverageOfChildren(int * colorArray = NULL);
    void setRandomColor(int minimumBrightness);
    
    unsigned char *octalCode;
    unsigned char color[4];
    VoxelNode *children[8];
    unsigned char childMask;
};

#endif /* defined(__hifi__VoxelNode__) */