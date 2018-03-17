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
#include "assert.h"

typedef unsigned char	   U8;
typedef unsigned short      U16;
typedef unsigned long	   U32;
typedef unsigned long long  U64;
typedef float			   F32;
#define F32_MAX FLT_MAX
#define U32_MAX ULONG_MAX
#define ArraySize(array) sizeof(array) / sizeof(array[0]);

#include "ray_math.h"
#include "ray_bmp.h"
#include "ray_bmp.cpp"

#include "ray_world.h"
#include "ray_tracing.h"

#define DEBUG_DISABLE_PARALLEL_THREADING 0
#include "ray_os.cpp"

#define DEBUG_SELFINTERSECTION 1
#define DEBUG_DISABLE_SHADING  0
#include "ray_tracing.cpp"

/*
Defines
*/
#define ResultFile "result.bmp"

static void CalculateCameraAxis(V3 cameraP,
                                V3* cameraX, V3* cameraY, V3* cameraZ) {
    *cameraZ = Normalize(cameraP);
    *cameraX = Normalize(Cross({0,0,1}, *cameraZ));
    *cameraY = Normalize(Cross(*cameraZ, *cameraX));
}

int main() {
    printf("Start ray tracing . . .\n");
    
    BMP_Image image;
    InitBMPImage(&image,
                 1280, 720);
    
    Material materials[] 
    {
        {{0.2,0.6,0.8}, 0,    1},
        {{0.8,0.8,0.8}, 0,    1},
        {{0,1,0},       0,    0.4f},
        {{0,0,1},       1,    0.0f},
        {{1,1,1},       0,    1},
        {{0,0,0},       0,    1},
        {{0,0,1},       0.5f, 0.5f}
    };
    
    Plane planes[] = 
    {
        {0, {0,0,1}, {0,0,0}, 5, 4}
    };
    
    Sphere spheres[] = {
        {1, {-2,0,1}, 1, 2},
        {2, {0,0,1},  1, 3},
        {3, {2,0,1},  1, 6}
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
    
    Options maxOptions;
    maxOptions.saaMode = SAAMode_SSAA;
    maxOptions.samplesToTake = 16;
    maxOptions.samplesPerDim = 4;
    maxOptions.samplesPerShading = 256;
    maxOptions.sampleRegionSize = 0.5;
    
    Options devOptions;
    devOptions.saaMode = SAAMode_SSAA;
    devOptions.samplesToTake = 4;
    devOptions.samplesPerDim = 2;
    devOptions.samplesPerShading = 128;
    devOptions.sampleRegionSize = 0.5;
    
    
    Options devOptionsMinimal;
    devOptionsMinimal.saaMode = SAAMode_SSAA;
    devOptionsMinimal.samplesToTake = 1;
    devOptionsMinimal.samplesPerDim = 1;
    devOptionsMinimal.samplesPerShading = 1;
    devOptionsMinimal.sampleRegionSize = 0.5;
    
    
    Options options = devOptionsMinimal;
    options = devOptions;
    options = maxOptions;
    
    U32 imageHeight; 
    U32 imageWidth;
    GetDimensions(&image, &imageHeight, &imageWidth);
    
    U32* packedPixelData = GetPackedPixelData(&image);
    
    //NOTE(ans): setup camera looking at origin
    V3 cameraP = {0, -20, 5};
    V3 cameraX, cameraY, cameraZ;
    CalculateCameraAxis(cameraP, &cameraX, &cameraY, &cameraZ);
    
    //NOTE(ans): setup film area to shoot rays through
    F32 distToCamera = 1;
    V3 filmC = cameraP - (cameraZ * distToCamera);
    
    //NOTE(ans): assumes that the the max of width and height is 1 in vp space
    //TODO: handle that height is greater then width 
    F32 filmWidth = 1;
    F32 filmHeight = (F32)imageHeight / (F32)imageWidth;  
    F32 filmWidthHalf = filmWidth * 0.5f;
    F32 filmHeightHalf = filmHeight * 0.5f;
    
    
    SAAData saaData;
    CalculateSAAData(options.saaMode,
                     filmWidth, filmHeight,
                     imageWidth, imageHeight,
                     cameraX, cameraY,
                     &saaData);
    
    U64 startTimeStamp = GetTimeStamp();
    U64 startTicks = GetCPUTicks(); 
    
    RayTraceImage(imageHeight, imageWidth,
                  cameraP, cameraX, cameraY,
                  filmWidthHalf, filmHeightHalf, filmC,
                  &world,
                  packedPixelData,
                  &options,
                  &saaData);
    
    U64 endTicks = GetCPUTicks();
    U64 endTimeStamp = GetTimeStamp();
    
    WriteBMPImage(&image, ResultFile);
    
    U64 microseconds = endTimeStamp - startTimeStamp;
    printf("\n-------------------------------------\n");
    printf("Performance:\n");
    printf("Ticks:        %llu\n", endTicks - startTicks);
    printf("Microseconds: %llu\n", microseconds);
    printf("Seconds:      %llu\n", (microseconds / 1000) / 1000);
    printf("-------------------------------------\n");
    
    printf("Finished ray tracing . . .\n");
    return 0;
}