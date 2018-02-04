/*
Note:

Coordinate Systems:
- Scene: x -> right, y -> depth, z -> up
- Camera: x -> right, y -> up, z -> back, x and y correspond to the dimension of the 
produced image, z is not really used

Camera System:
- Camera: camera define the setup of the viewport CameraP is the ray orign
- Viewport: defines the relation of film to the actual image
- Film: image representation in the scene through which rays are projected

*/


/*
Cl stuff
*/
#define _CRT_SECURE_NO_WARNINGS 1


/*
Includes
*/
#include "stdio.h"
#include "stdlib.h" 
#include "string.h"
#include "float.h"

/*
 Core definitions
*/
#define Internal static
typedef unsigned char	   U8;
typedef unsigned short      U16;
typedef unsigned long	   U32;
typedef float			   F32;

#define F32_MAX FLT_MAX

#define ArraySize(array) sizeof(array) / sizeof(array[0]);

/*
Own includes
*/
#include "ray_math.h"
#include "ray_bmp.h"
#include "ray_bmp.cpp"

#include "ray_world.h"
#include "ray_tracing.cpp"

/*
Defines
*/
#define ResultFile "result.bmp"

Internal void CalculateCameraAxis(V3 cameraP,
                                  V3* cameraX, V3* cameraY, V3* cameraZ) {
    *cameraZ = Normalize(cameraP);
    *cameraX = Normalize(Cross({0,0,1}, *cameraZ));
    *cameraY = Normalize(Cross(*cameraZ, *cameraX));
}

int main() {
    printf("Start ray tracing . . .\n");
    
    BMP_Image image;
    InitBMPImage(&image,
                 1600, 900);
    
    Material materials[] 
    {
        {{0.2,0.6,0.8}},
        {{0.8,0.8,0.8}},
        {{0,1,0}},
        {{0,0,1}}
    };
    
    Plane planes[] = 
    {
        {0, {0,0,1}, {0,0,0}, 1}
    };
    
    Sphere spheres[] = {
        {1, {0,0,1}, 1, 2},
        {2, {2,0,2}, 1, 3}
    };
    
    Light lights[] = {
        {{1,1,1},   0.5, LightType_Directional, {-0.5, 0, 1}},
        {{1,1,1},   500, LightType_Point,       {3,  0, 5}},
        {{1,1,0.4}, 500, LightType_Point,       {-3, 0, 6}}
    };
    
    World world;
    world.materials = materials;
    world.planes = planes;
    world.planeCount = ArraySize(planes);
    world.spheres = spheres;
    world.sphereCount = ArraySize(spheres);
    world.lights = lights;
    world.lightCount = ArraySize(lights);
    
    U32 imageHeight; 
    U32 imageWidth;
    GetDimensions(&image, &imageHeight, &imageWidth);
    
    U32* packedPixelData = GetPackedPixelData(&image);
    
    V3 cameraP = {0, -20, 5};
    V3 cameraX, cameraY, cameraZ;
    CalculateCameraAxis(cameraP, &cameraX, &cameraY, &cameraZ);
    
    F32 distToCamera = 1;
    V3 filmC = cameraP - (cameraZ * distToCamera);
    
    //Note(ans): assumes that the the max of width and height is 1 in vp space
    //TODO: handle that height is greater then width 
    F32 filmWidth = 1;
    F32 filmHeight = (F32)imageHeight / (F32)imageWidth;  
    
    F32 filmWidthHalf = filmWidth * 0.5f;
    F32 filmHeightHalf = filmHeight * 0.5f;
    
    for(U32 imageY = 0; imageY < imageHeight; ++imageY) {
        F32 viewPortY = - 1 + 2 * ((F32)imageY / (F32)imageHeight);
        
        for(U32 imageX = 0; imageX < imageWidth; ++imageX) {
            F32 viewPortX = - 1 + 2 * ((F32)imageX / (F32)imageWidth);
            
            
            V3 filmXOffset = cameraX * (viewPortX * filmWidthHalf);
            V3 filmYOffset = cameraY * (viewPortY * filmHeightHalf);
            
            V3 filmP = filmC + filmXOffset + filmYOffset;
            
            V3 rayOrigin = cameraP;
            V3 rayDirection = Normalize(filmP - cameraP);
            
            V3 rayCastResult = RayTrace(rayOrigin, rayDirection,
                                        &world);
            
            U32 pixelIndex = imageY * imageWidth + imageX;
            packedPixelData[pixelIndex] = PackColor(rayCastResult); 
        }
        
        if((imageY % 64) == 0) {
            F32 progress = (F32)imageY / (F32)imageHeight;
            printf("\rProgress %0.2f ", progress);
            fflush(stdout);
        }
    }
    
    WriteBMPImage(&image, ResultFile);
    
    
    printf("\nFinished ray tracing . . .\n");
    return 0;
}