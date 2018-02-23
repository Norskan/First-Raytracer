enum SAAMode {
    SAAMode_None,
    SAAMode_SSAA
};

struct Options {
    SAAMode saaMode;
    
    //saa
    U32 samplesToTake;
    U32 samplesPerDim;
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
