//
//  RenderUtil.h
//  interface
//
//  Created by Andrzej Kapolka on 8/15/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#ifndef __interface__RenderUtil__
#define __interface__RenderUtil__

/// Renders a quad from (-1, -1, 0) to (1, 1, 0) with texture coordinates from (sMin, 0) to (sMax, 1).
void renderFullscreenQuad(float sMin = 0.0f, float sMax = 1.0f);

#endif /* defined(__interface__RenderUtil__) */
