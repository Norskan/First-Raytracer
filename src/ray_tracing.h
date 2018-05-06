enum SAAMode {
    SAAMode_None,
    SAAMode_SSAA
};

struct Options {
    // Anti Aliasing
    SAAMode saaMode;
    U32 samplesToTake;
    U32 samplesPerDim;
    
    // Soft Shadow
    U32 samplesPerShading;
    F32 sampleRegionSize;
    V3* sampleDataBuffer;
};

struct ShootRayResult {
    U32 hit;
    U32 hitMatIndex;
    V3 hitNormal;
    V3 hitPoint;
    
    //debug
    U32 hitId;
    char* hitName;
};

struct RayTraceData {
    U32 imageHeight;
};


struct SAAData {
    V3 sampleRegionX;
    V3 sampleRegionY;
};

struct V3Array_64{
    inline V3& operator[](U32 i) { return values[i]; }
    V3 values[64];
};

struct PixelSamplingPoints {
    V3Array_64 points;
    U32 count;
};

struct RayTraceSectionRowData {
    U32 imageHeight;
    U32 rowHeight; 
    U32 rowWidth; 
    U32 rowYStart;
    
    V3 cameraP; 
    V3 cameraX;
    V3 cameraY;
    
    
    F32 filmWidthHalf; 
    F32 filmHeightHalf; 
    V3 filmC;
    
    World* world;
    Options options; 
    SAAData saaData;
    U32* packedPixelData;
    
    RandomSeries series;
    U32 randomCirclePointCount;
    V3* randomCirclePoints;
};
