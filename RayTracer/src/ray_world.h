/*
remove ids by merging objects toggeter in one single
structure
*/

struct Material {
    V3 color;
};

struct Plane {
    U32 id;
    V3 n;
    V3 p;
    
    U32 matIndex;
    U32 secMatIndex;
};

struct Sphere {
    U32 id;
    V3 p;
    F32 r;
    
    U32 matIndex;
};

enum LightType {
    LightType_Directional,
    LightType_Point
};

struct Light {
    V3 color;
    F32 intensity;
    
    LightType type;
    union {
        struct {
            V3 invertedDirection;
        } d;
        
        struct {
            V3 origin;
        } p;
    };
};

struct World {
    Material* materials;
    
    Plane* planes;
    U32 planeCount;
    
    Sphere* spheres;
    U32 sphereCount; 
    
    Light* lights;
    U32 lightCount;
    
};