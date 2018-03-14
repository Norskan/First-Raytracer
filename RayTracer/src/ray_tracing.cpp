static inline void RayTraceObjects(V3 rayOrigin, V3 rayDirection,
                                   World* world,
                                   F32 traceMaxDistance,
                                   ShootRayResult* result) {
    F32 tolerance = 0.01;
    
    F32 hitDistance = traceMaxDistance;
    V3 hitNormal = {};
    U32 hitMatIndex = 0;
    
    Plane* planes = world->planes;
    U32 planeCount = world->planeCount;
    for(U32 planeIndex = 0; 
        planeIndex < planeCount; 
        ++planeIndex) {
        Plane currentPlane = planes[planeIndex];
        
        F32 divisor = Inner(rayDirection, currentPlane.n);
        
        if((divisor < -tolerance) || (divisor > tolerance)) {
            F32 divident = Inner(currentPlane.p, currentPlane.n) - Inner(rayOrigin, currentPlane.n);
            F32 t = divident / divisor;
            
            if(t > tolerance &&  t < hitDistance) {
                hitDistance = t;
                
                
                V3 hitPoint = rayOrigin + (rayDirection * hitDistance);
                V3 toHitPoint = hitPoint - currentPlane.p;
                V2 project2D = CreateV2(hitPoint);
                
                bool checkered = ((U32)((U32)project2D.x ^ (U32)project2D.y)) & 1;
                
                //NOTE(ans): 
                // because our coordinates go from -Inv to +Inv we get a invalid checker pattern
                // at 0.
                // this solution can be simplified by transforming the coordinates to values between
                // 0 and +Inv but for that the scene size needs to be known which it is not at the moment
                if(project2D.y < 0) {
                    checkered = !checkered;
                }
                
                if(project2D.x < 0) {
                    checkered = !checkered;
                }
                
                if(checkered) {
                    hitMatIndex = currentPlane.matIndex;
                } else {
                    hitMatIndex = currentPlane.secMatIndex;
                }
                
                hitNormal = currentPlane.n;
                
                result->hitName = "Plane";
                result->hitId = currentPlane.id;
                result->hit = 1;
            }
        }
    }
    
    Sphere* spheres = world->spheres;
    U32 sphereCount = world->sphereCount;
    
    for(U32 sphereIndex = 0; 
        sphereIndex < sphereCount; 
        ++sphereIndex) {
        
        Sphere currentSphere  = spheres[sphereIndex];
        V3 rayOriginRelativSphere = rayOrigin - currentSphere.p;
        
        
        F32 a = Inner(rayDirection, rayDirection);
        F32 b = 2*Inner(rayDirection, rayOriginRelativSphere);
        F32 c = Inner(rayOriginRelativSphere, rayOriginRelativSphere) - (currentSphere.r * currentSphere.r);
        F32 rootTerm = b*b - 4*a*c;
        
        if (rootTerm > tolerance) { 
            F32 rootValue = SquareRoot(rootTerm);
            
            F32 tPlus = (-b + rootValue) / 2*a;
            F32 tMinus = (-b - rootValue) / 2*a;
            
            F32 distance = tPlus;
            if(tMinus < distance) {
                distance = tMinus;
            }
            
            if(distance > tolerance) {
                if(distance < hitDistance) {
                    hitDistance = distance;
                    hitMatIndex = currentSphere.matIndex;
                    
                    V3 hitPoint = rayOrigin + (rayDirection * hitDistance);
                    hitNormal = Normalize(hitPoint - currentSphere.p);
                    
                    
                    result->hitName = "Sphere";
                    result->hitId = currentSphere.id;
                    result->hit = 1;
                }
            }
        }
    }
    
    result->hitMatIndex = hitMatIndex;
    result->hitNormal = hitNormal;
    result->hitPoint = rayOrigin+(rayDirection*hitDistance);
}

static inline void GenerateLightSamples(V3* result, U32 resultCount,
                                        V3 hitNormal, V3 hitPoint,
                                        F32 sampleSize) {
    F32 shadowBias = 0.0001f;
    F32 lowerBound = 0.0000001f;
    
    V3 sampleOrigin = hitPoint + hitNormal * shadowBias;
    
    V3 v;
    
    if(lowerBound > hitNormal.x && lowerBound > hitNormal.y) {
        v.x = hitNormal.z;
        v.y = 0;
        v.z = -hitNormal.x;
    } else {
        v.x = -hitNormal.y;
        v.y = hitNormal.x;
        v.z = 0;
    }
    
    V3 w = Cross(hitNormal, v);
    
    
    F32 radius = sampleSize;
    for(U32 resultIndex = 0; resultIndex < resultCount; ++ resultIndex) {
        //generate 2d sample in circle
        F32 angle = RandUnitF32() * TAU;
        F32 r = radius * SquareRoot(RandUnitF32());
        
        F32 x = r * (F32)cos(angle);
        F32 y = r * (F32)sin(angle);
        
        //transform samples to hit normal system
        V3 samplePoint = (v * x) + (w * y);
        
        //transform samples to hitpoint
        samplePoint = samplePoint + sampleOrigin;
        
        result[resultIndex] = samplePoint;
    }
    
}

static inline V3 RayTraceLights(World* world,
                                U32 objectId, V3 materialColor, 
                                V3 hitNormal, V3 hitPoint,
                                U32 lightSamplePointCount, V3* lightSampleDataBuffer, F32 sampleRegion) {
    V3 resultColor = {};
    
    Light* lights = world->lights;
    U32 lightCount = world->lightCount;
    
    
    F32 lightContribution = 1.0f / lightCount;
    for(U32 lightIndex = 0; 
        lightIndex < lightCount;
        ++lightIndex) {
        Light currentLight = lights[lightIndex];
        V3 colorShading = {};
        
        GenerateLightSamples(lightSampleDataBuffer, lightSamplePointCount, 
                             hitNormal, hitPoint,
                             sampleRegion);
        
        F32 lightSampleContribution = 1.0f / lightSamplePointCount;
        for(U32 lightSamplePointIndex = 0; lightSamplePointIndex < lightSamplePointCount; ++lightSamplePointIndex){
            V3 lightRayOrigin = lightSampleDataBuffer[lightSamplePointIndex];
            
            V3 lightRayDirection = {};
            V3 lightIntensity = {1,1,1};
            F32 traceMaxDistance = F32_MAX;
            switch(currentLight.type) {
                case(LightType_Directional):  {
                    lightRayDirection = Normalize(currentLight.d.invertedDirection);
                    
                    F32 shading = Inner(hitNormal, Normalize(currentLight.d.invertedDirection));
                    shading= Max(shading, 0);
                    
                    lightIntensity = currentLight.color * currentLight.intensity * shading;
                } break;
                case(LightType_Point): {
                    V3 direction = currentLight.p.origin - lightRayOrigin;
                    F32 rSquare = Inner(direction);
                    lightRayDirection = Normalize(direction);
                    traceMaxDistance = SquareRoot(rSquare);
                    
                    V3 fallOff = (currentLight.color*currentLight.intensity) / (4.0f*PI*rSquare);
                    
                    F32 shading = Inner(hitNormal, Normalize(lightRayDirection));
                    shading = Max(shading, 0);
                    
                    lightIntensity = fallOff * shading;
                } break;
            }
            
            ShootRayResult lightResult = {};
            RayTraceObjects(lightRayOrigin, lightRayDirection,
                            world, 
                            traceMaxDistance,
                            &lightResult);
            
            bool selfIntersect = lightResult.hitId == objectId;
            F32 visible = (F32)(!lightResult.hit || (lightResult.hit && selfIntersect));
            
            
            colorShading = colorShading + (lightIntensity  * visible * lightSampleContribution);
            
        }
        
        resultColor = resultColor + materialColor * colorShading * lightContribution;
    }
    
    return resultColor;
}

static inline V3 CalculateColor(V3 rayOrigin, V3 rayDirection,
                                World* world,
                                U32 lightSamplePointCount, V3* lightSampleDataBuffer, F32 sampleRegion,
                                U32 depth, U32 lastHitId) {
    Material* materials = world->materials;
    
    
    ShootRayResult result = {};
    RayTraceObjects(rayOrigin,
                    rayDirection,
                    world,
                    F32_MAX,
                    &result);
    
    if(result.hit) {
        
#if DEBUG_SELFINTERSECTION 
        if(result.hitId == lastHitId) {
            DebuggerBreak();
        }
#endif
        
        
        Material material = materials[result.hitMatIndex];
        
        V3 shadedColor = RayTraceLights(world,
                                        result.hitId, material.color, 
                                        result.hitNormal, result.hitPoint,
                                        lightSamplePointCount, lightSampleDataBuffer, sampleRegion);
        V3 color = shadedColor;
        
        
        
        if(depth < 8) {
            //Note(ans):
            //offset origin by an offest, because intersections can under the actual objects because
            //of rounding errors
            V3 newRayOrigin = result.hitPoint;
            
            V3 specularDirection = VectorReflected(rayDirection, result.hitNormal);
            
            V3 unitSphereOrigin = newRayOrigin + result.hitNormal;
            V3 randomPoint = RandomPointInUnitSphere(unitSphereOrigin);
            V3 diffuseDirection = randomPoint - newRayOrigin;
            
            V3 specularColor = CalculateColor(newRayOrigin, specularDirection,
                                              world,
                                              lightSamplePointCount, lightSampleDataBuffer, sampleRegion,
                                              depth + 1, result.hitId);
            
#if 0            
            V3 diffuseColor = CalculateColor(newRayOrigin, diffuseDirection,
                                             world,
                                             lightSamplePointCount, lightSampleDataBuffer, sampleRegion,
                                             depth + 1, result.hitId);
#endif
            
            V3 diffuseColor = color;
            
            V3 reflectionColor = Lerp(diffuseColor, material.reflection, specularColor);  
            
            color = Lerp(reflectionColor, material.absorbtion, color);
        }
        
        return color;
    } else {
        return materials[result.hitMatIndex].color;
    }
}

static inline V3 RayTraceWorld(V3 rayOrigin, V3 rayDirection,
                               World* world,
                               U32 lightSamplePointCount, V3* lightSampleDataBuffer, F32 sampleRegion) {
#if 0    
    Material* materials = world->materials;
    V3 resultColor = {1,1,1};
    
    U32 maxBounceRays = 8;
    
    
    for(U32 bounceRayIndex = 0; bounceRayIndex < maxBounceRays; ++bounceRayIndex) {
        ShootRayResult result = {};
        RayTraceObjects(rayOrigin,
                        rayDirection,
                        world,
                        F32_MAX,
                        &result);
        
        if(result.hit) {
            V3 materialColor = materials[result.hitMatIndex].color;
            
            V3 shadedColor = RayTraceLights(world,
                                            result.hitId, materialColor, 
                                            result.hitNormal, result.hitPoint,
                                            lightSamplePointCount, lightSampleDataBuffer, sampleRegion);
            
            if(materials[result.hitMatIndex].reflects) {
                resultColor = Lerp(resultColor, 0.5f, shadedColor);
                
                rayOrigin = result.hitPoint;
                rayDirection = VectorReflected(rayDirection, result.hitNormal);
            } 
            else {
                resultColor = resultColor * shadedColor;
            }
            
        } else {
            resultColor = materials[result.hitMatIndex].color;
            break;
        }
    }
    
    return resultColor;
#else
    
    return CalculateColor(rayOrigin, rayDirection,
                          world,
                          lightSamplePointCount, lightSampleDataBuffer, sampleRegion,
                          0, U32_MAX);
    
#endif
}

static inline  PixelSamplingPoints CalculatePixelSamplingPoints(V3 bl, 
                                                                V3 sampleRegionX, V3 sampleRegionY,
                                                                U32 samplesToTake, U32 samplesPerDim) {
    //grid uniform distribution
    PixelSamplingPoints result;
    result.count = samplesToTake;
    
    V3 sampleOffsetX = sampleRegionX / (F32)samplesPerDim;
    V3 sampleOffsetY = sampleRegionY / (F32)samplesPerDim;
    
    V3 sampleOffsetXHalf = sampleOffsetX / 2;
    V3 sampleOffsetYHalf = sampleOffsetY / 2;
    
    V3 sampleBl = bl + sampleOffsetXHalf + sampleOffsetYHalf;
    
    U32 sampleCount = 0;
    for(U32 x = 0; x < samplesPerDim; ++x) {
        V3 sampleX = sampleOffsetX * (F32)x;
        for(U32 y = 0; y < samplesPerDim; ++y) {
            V3 sampleY = sampleOffsetY * (F32)y;
            
            V3 sample = sampleBl + sampleX + sampleY;
            result.points[sampleCount] = sample;
            
            ++sampleCount;
        }
    }
    
    return result;
}

static inline void CalculateSAAData(SAAMode mode, 
                                    F32 filmWidth, F32 filmHeight,
                                    U32 imageWidth, U32 imageHeight,
                                    V3 cameraX, V3 cameraY,
                                    SAAData* data) {
    if(mode == SAAMode_SSAA) {
        V3 pixelSize;
        pixelSize.x = filmWidth / imageWidth;
        pixelSize.y = filmHeight / imageHeight;
        
        V3 sampleRegionX = cameraX * pixelSize.x;
        data->sampleRegionX = sampleRegionX;
        
        V3 sampleRegionY = cameraY * pixelSize.y;
        data->sampleRegionY = sampleRegionY;
    }
}

static inline void RayTraceImage(U32 imageHeight, U32 imageWidth,
                                 V3 cameraP, V3 cameraX, V3 cameraY,
                                 F32 filmWidthHalf, F32 filmHeightHalf, V3 filmC,
                                 World* world,
                                 U32* packedPixelData,
                                 Options* i_options,
                                 SAAData* i_saaData) {
    
    SAAData ssaData = *i_saaData;
    Options options = *i_options;
    SAAMode saaMode = options.saaMode;
    
    for(U32 imageY = 0; imageY < imageHeight; ++imageY) {
        F32 viewPortY = - 1 + 2 * ((F32)imageY / (F32)imageHeight);
        
        for(U32 imageX = 0; imageX < imageWidth; ++imageX) {
            F32 viewPortX = - 1 + 2 * ((F32)imageX / (F32)imageWidth);
            
            V3 filmXOffset = cameraX * (viewPortX * filmWidthHalf);
            V3 filmYOffset = cameraY * (viewPortY * filmHeightHalf);
            
            V3 filmP = filmC + filmXOffset + filmYOffset;
            
            V3 pixel = {};
            
            switch(saaMode) {
                case(SAAMode_None): {
                    V3 rayOrigin = cameraP;
                    V3 rayDirection = Normalize(filmP - cameraP);
                    
                    pixel = RayTraceWorld(rayOrigin, rayDirection,
                                          world,
                                          options.samplesPerShading, options.sampleDataBuffer, options.sampleRegionSize);
                } break;
                case(SAAMode_SSAA): {
                    PixelSamplingPoints samplingPoints = CalculatePixelSamplingPoints(filmP, 
                                                                                      ssaData.sampleRegionX, ssaData.sampleRegionY, 
                                                                                      options.samplesToTake, options.samplesPerDim);
                    
                    V3Array_64 sampels;
                    for(U32 sampleIndex = 0; sampleIndex < samplingPoints.count; ++sampleIndex) {
                        V3 samplePoint = samplingPoints.points[sampleIndex];
                        
                        V3 rayOrigin = cameraP;
                        V3 rayDirection = Normalize(samplePoint - cameraP);
                        
                        V3 traceResult = RayTraceWorld(rayOrigin, rayDirection,
                                                       world,
                                                       options.samplesPerShading, options.sampleDataBuffer, options.sampleRegionSize);
                        
                        sampels[sampleIndex] = traceResult;
                    }
                    
                    //Average Filter
                    F32 contribution = 1.0f / samplingPoints.count;
                    for(U32 sampleIndex = 0;
                        sampleIndex < options.samplesToTake;
                        sampleIndex++) {
                        pixel = pixel + (sampels[sampleIndex] * contribution);
                    }
                    
                } break;
            }
            
            U32 pixelIndex = imageY * imageWidth + imageX;
            packedPixelData[pixelIndex] = PackColor(pixel); 
        }
        
        if((imageY % 64) == 0) { 
            F32 progress = (F32)imageY / (F32)imageHeight; 
            printf("\rProgress %0.2f ", progress); 
            fflush(stdout); 
        }
    }
}