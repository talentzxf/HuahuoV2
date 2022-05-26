//
// Created by VincentZhang on 5/26/2022.
//

#include "Image.h"
#include "Utilities/BitUtility.h"

int CalculateMipMapCount3D(int width, int height, int depth)
{
    //Assert (IsPowerOfTwo (width) && IsPowerOfTwo (height) && IsPowerOfTwo (depth));

    // Mip-levels for non-power-of-two textures follow OpenGL's NPOT texture rules: size is divided
    // by two and floor'ed. This allows just to use same old code I think.

    int minSizeLog2 = HighestBit(width);
    minSizeLog2 = std::max(minSizeLog2, HighestBit(height));
    minSizeLog2 = std::max(minSizeLog2, HighestBit(depth));

    Assert(!((width >> minSizeLog2) < 1 && (height >> minSizeLog2) < 1 && (depth >> minSizeLog2) < 1));
    Assert(!((width >> minSizeLog2) > 1 && (height >> minSizeLog2) > 1 && (depth >> minSizeLog2) < 1));

    return minSizeLog2 + 1;
}