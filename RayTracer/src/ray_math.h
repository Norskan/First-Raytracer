#include "math.h"

/*
Constants
*/
#define PI 3.14159265359f
#define TAU 6.28318530718f

/*
Rand
*/

#include "time.h" //time
static inline void InitRand() {
    srand((unsigned int)time(NULL));
}

//returns values between 0 and 1
static inline F32 RandUnitF32() {
    F32 result;
    
    result = (F32)rand() / (F32)RAND_MAX;
    
    return result;
}

/*
F32
*/
static inline F32 Pow(F32 x, F32 y) {
    F32 result;
    
    result = (F32)pow((double)x, (double)y);
    
    return result;
}

static inline F32 SquareRoot(F32 v) {
    F32 result;
    
    result = (F32)sqrt((double)v);
    
    return result;
}

static inline F32 Max(F32 v1, F32 v2) {
    F32 result = v1;
    
    if(result < v2) {
        result = v2;
    }
    
    return result;
}

/*
V3
*/
union V3 {
    struct {
        F32 x, y, z;
    };
    
    struct {
        F32 r, g, b;
    };
};

static inline V3 Invert(V3 v) {
    V3 result;
    
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    
    return result;
}

V3 inline operator+(V3 v1, V3 v2) {
    V3 result;
    
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    
    return result;
}

V3 operator-(V3 v1, V3 v2) {
    V3 result;
    
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    
    return result;
}

V3 operator-(V3 v, F32 c) {
    V3 result;
    
    result.x = v.x - c;
    result.y = v.y - c;
    result.z = v.z - c;
    
    return result;
}

V3 operator*(V3 v, F32 scalar) {
    V3 result;
    
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    result.z = v.z * scalar;
    
    return result;
}


V3 operator*(F32 scalar, V3 v) {
    return v * scalar;
}

V3 operator*(V3 v1, V3 v2) {
    V3 result;
    
    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;
    
    return result;
}

V3 operator/(V3 v, F32 divisor) {
    V3 result;
    
    result.x = v.x / divisor;
    result.y = v.y / divisor;
    result.z = v.z / divisor;
    
    return result;
}

static inline F32 Inner(V3 v1, V3 v2) {
    F32 result;
    
    result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; 
    
    return result;
}

static inline F32 Inner(V3 v) {
    return Inner(v, v);
}

static inline V3 Cross(V3 v1, V3 v2) {
    V3 result;
    
    result.x = v1.y*v2.z - v1.z*v2.y;
    result.y = v1.z*v2.x - v1.x*v2.z;
    result.z = v1.x*v2.y - v1.y*v2.x;
    
    return result;
}

static inline V3 Hadamard(V3 v1, V3 v2) {
    V3 result;
    
    result.x = v1.x * v2.x;
    result.y = v1.y * v2.y;
    result.z = v1.z * v2.z;
    
    return result;
}

static inline V3 Normalize(V3 v) {
    V3 result;
    
    F32 innerProduct = Inner(v, v);
    F32 length = SquareRoot(innerProduct);
    
    result = v / length;
    
    return result;
}

static inline F32 LengthRoot(V3 v) {
    F32 result;
    
    F32 innerProduct = Inner(v, v);
    result = SquareRoot(innerProduct);
    
    return result;
}


static inline F32 Length(V3 v) {
    F32 result;
    
    result = Inner(v, v);
    
    return result;
}

static inline V3 VectorReflected(V3 v, V3 n) {
    V3 result;
    
    F32 l = Inner(v, n);
    V3 _n = n * l;
    
    V3 _n2 = _n * 2;
    
    result = v - _n2;
    
    return result;
}


static inline V3 Lerp(V3 a, F32 t, V3 b) {
    V3 result;
    
    result = (a * (1 - t)) + (b * t);
    
    return result;
}

static inline V3 RandomUnitVector() {
    V3 result;
    
    result.x = RandUnitF32();
    result.y = RandUnitF32();
    result.z = RandUnitF32();
    
    return result;
}

static inline V3 RandomPointInUnitSphere(V3 origin) {
    V3 result;
    
    V3 randomPoint;
    
    do {
        V3 v;
        v.x = RandUnitF32();
        v.y = RandUnitF32();
        v.z = RandUnitF32();
        
        randomPoint = 2.0f * v - 1.0f;
    } while(LengthRoot(randomPoint) >= 1.0f);
    
    result = randomPoint;
    
    return result;
}

/*
V2
*/

struct V2 {
    F32 x;
    F32 y;
};

static inline V2 CreateV2(V3 v) {
    V2 result;
    
    result.x = v.x;
    result.y = v.y;
    
    return result;
}

V2 operator*(V2 v, F32 scalar) {
    V2 result;
    
    result.x = v.x * scalar;
    result.y = v.y * scalar;
    
    return result;
}


V2 operator+(V2 v, F32 c) {
    V2 result;
    
    result.x = v.x + c;
    result.y = v.y + c;
    
    return result;
}


