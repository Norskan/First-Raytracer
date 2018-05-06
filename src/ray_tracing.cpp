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
    
    // optimice performance
    // - a only needs to be calculated once per ray
    // - c part two only needs to be calculated once per sphere
    
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

static void GenerateLightSamples(V3* result, U32 resultCount,
                                 V3 hitNormal, V3 hitPoint,
                                 RandomSeries* series,
                                 V3* randomCirclePoints,
                                 U32 randomCirclePointCount) {
    
    
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
    
    for(U32 resultIndex = 0; resultIndex < resultCount; ++ resultIndex) {
        
        U32 randomSampleIndex = RandomU32(series, randomCirclePointCount);
        V3 randomPoint = randomCirclePoints[randomSampleIndex];
        
        //transform samples to hit normal system
        V3 samplePoint = (v * randomPoint.x) + (w * randomPoint.y);
        
        //transform samples to hitpoint
        samplePoint = samplePoint + sampleOrigin;
        
        result[resultIndex] = samplePoint;
    }
}

static V3 RayTraceLights(World* world,
                         U32 objectId, V3 materialColor, 
                         V3 hitNormal, V3 hitPoint,
                         U32 lightSamplePointCount, V3* lightSampleDataBuffer,
                         RandomSeries* series,
                         V3* randomCirclePoints,
                         U32 randomCirclePointCount) {
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
                             series,
                             randomCirclePoints,
                             randomCirclePointCount);
        
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

static V3 CalculateColor(V3 rayOrigin, V3 rayDirection,
                         World* world,
                         U32 lightSamplePointCount, V3* lightSampleDataBuffer,
                         U32 depth, U32 lastHitId,
                         RandomSeries* series,
                         V3* randomCirclePoints,
                         U32 randomCirclePointCount) {
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
        
#if DEBUG_DISABLE_SHADING     
        V3 color = material.color;
#else
        V3 shadedColor = RayTraceLights(world,
                                        result.hitId, material.color, 
                                        result.hitNormal, result.hitPoint,
                                        lightSamplePointCount, lightSampleDataBuffer,
                                        series,
                                        randomCirclePoints,
                                        randomCirclePointCount);
        V3 color = shadedColor;
        
#endif
        
        
        if(depth < 8) {
            //Note(ans):
            //offset origin by an offest, because intersections can under the actual objects because
            //of rounding errors
            V3 newRayOrigin = result.hitPoint;
            
            V3 specularDirection = VectorReflected(rayDirection, result.hitNormal);
            V3 specularColor = CalculateColor(newRayOrigin, specularDirection,
                                              world,
                                              lightSamplePointCount, lightSampleDataBuffer,
                                              depth + 1, result.hitId,
                                              series,
                                              randomCirclePoints,
                                              randomCirclePointCount);
            
#if 0       
            V3 unitSphereOrigin = newRayOrigin + result.hitNormal;
            V3 randomPoint = RandomPointInUnitSphere(unitSphereOrigin);
            V3 diffuseDirection = randomPoint - newRayOrigin;
            
            V3 diffuseColor = CalculateColor(newRayOrigin, diffuseDirection,
                                             world,
                                             lightSamplePointCount, lightSampleDataBuffer, sampleRegion,
                                             depth + 1, result.hitId,
                                             series,
                                             randomCirclePoints,
                                             randomCirclePointCount);
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

static PixelSamplingPoints CalculatePixelSamplingPoints(V3 bl, 
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

static void CalculateSAAData(SAAMode mode, 
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

static void RayTraceSectionRow(RayTraceSectionRowData* dataPointer) {
    RayTraceSectionRowData data = *dataPointer;
    Options options = data.options;
    SAAData saaData = data.saaData;
    U32 rowYEnd = data.rowYStart + data.rowHeight;
    
    for(U32 rowY = data.rowYStart; rowY < rowYEnd; ++rowY) {
        F32 viewPortY = - 1 + 2 * ((F32)rowY / (F32)data.imageHeight);
        
        for(U32 rowX = 0; rowX < data.rowWidth; ++rowX) {
            F32 viewPortX = - 1 + 2 * ((F32)rowX / (F32)data.rowWidth);
            
            V3 filmXOffset = data.cameraX * (viewPortX * data.filmWidthHalf);
            V3 filmYOffset = data.cameraY * (viewPortY * data.filmHeightHalf);
            
            V3 filmP = data.filmC + filmXOffset + filmYOffset;
            
            V3 pixel = {};
            
            switch(options.saaMode) {
                case(SAAMode_None): {
                    V3 rayOrigin = data.cameraP;
                    V3 rayDirection = Normalize(filmP - data.cameraP);
                    
                    pixel = CalculateColor(rayOrigin, rayDirection,
                                           data.world,
                                           options.samplesPerShading, options.sampleDataBuffer,
                                           0, U32_MAX,
                                           &data.series,
                                           data.randomCirclePoints,
                                           data.randomCirclePointCount);
                } break;
                case(SAAMode_SSAA): {
                    PixelSamplingPoints samplingPoints = CalculatePixelSamplingPoints(filmP, 
                                                                                      saaData.sampleRegionX, saaData.sampleRegionY, 
                                                                                      options.samplesToTake, options.samplesPerDim);
                    
                    V3Array_64 sampels;
                    for(U32 sampleIndex = 0; sampleIndex < samplingPoints.count; ++sampleIndex) {
                        V3 samplePoint = samplingPoints.points[sampleIndex];
                        
                        V3 rayOrigin = data.cameraP;
                        V3 rayDirection = Normalize(samplePoint - data.cameraP);
                        
                        V3 traceResult = CalculateColor(rayOrigin, rayDirection,
                                                        data.world,
                                                        options.samplesPerShading, options.sampleDataBuffer,
                                                        0, U32_MAX,
                                                        &data.series,
                                                        data.randomCirclePoints,
                                                        data.randomCirclePointCount);
                        
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
            
            U32 pixelIndex = rowY * data.rowWidth + rowX;
            data.packedPixelData[pixelIndex] = PackColor(pixel);
        }
    }
}

static void RayTraceImage(U32 imageHeight, U32 imageWidth,
                          V3 cameraP, V3 cameraX, V3 cameraY,
                          F32 filmWidthHalf, F32 filmHeightHalf, V3 filmC,
                          World* world,
                          U32* packedPixelData,
                          Options* i_options,
                          SAAData* i_saaData) {
    
    SAAData saaData = *i_saaData;
    Options options = *i_options;
    
    U32 cpuCores = GetCPUCores();
    U32 rowHeight = imageHeight / cpuCores;
    assert((rowHeight * cpuCores) == imageHeight);
    
    RayTraceSectionRowData* dataList = (RayTraceSectionRowData*)malloc(sizeof(RayTraceSectionRowData) * cpuCores);
    
    U32 randomCirclePointCount = 516;
    V3* randomCirclePoints = (V3*)malloc(sizeof(V3) * randomCirclePointCount);
    
    RandomSeries circleRandomSeries;
    circleRandomSeries.series = rand();
    F32 radius = options.sampleRegionSize;
    for(U32 randomCirclePointIndex = 0;
        randomCirclePointIndex < randomCirclePointCount; 
        ++randomCirclePointIndex) {
        
        F32 angle = RandUnitF32(&circleRandomSeries) * TAU;
        F32 r = radius * SquareRoot(RandUnitF32(&circleRandomSeries));
        
        V3 randomPoint;
        randomPoint.x = r * (F32)cos(angle);
        randomPoint.y = r * (F32)sin(angle);
        
        randomCirclePoints[randomCirclePointIndex] = randomPoint;
    }
    
    for(U32 cpuCoreIndex = 0; cpuCoreIndex < cpuCores; ++cpuCoreIndex) {
        U32 rowYStart = rowHeight * cpuCoreIndex;
        
        RayTraceSectionRowData rowData;
        rowData.imageHeight = imageHeight;
        rowData.rowHeight = rowHeight; 
        rowData.rowWidth = imageWidth; 
        rowData.rowYStart = rowYStart;
        rowData.cameraP = cameraP; 
        rowData.cameraX = cameraX; 
        rowData.cameraY = cameraY;;
        rowData.filmWidthHalf = filmWidthHalf; 
        rowData.filmHeightHalf = filmHeightHalf;
        rowData.filmC = filmC;
        rowData.world = world;
        rowData.options = options;
        rowData.saaData = saaData;
        rowData.packedPixelData = packedPixelData;
        rowData.series.series = rand();
        rowData.options.sampleDataBuffer = (V3*)malloc(sizeof(V3) * rowData.options.samplesPerShading);
        rowData.randomCirclePoints = randomCirclePoints;
        rowData.randomCirclePointCount = randomCirclePointCount;
        
        dataList[cpuCoreIndex] = rowData;
    }
    
    CreateMultipleThreadsAndWait((ThreadFunction)RayTraceSectionRow, 
                                 dataList,
                                 cpuCores);
    
#if 0        
    if((imageY % 64) == 0) { 
        F32 progress = (F32)imageY / (F32)imageHeight; 
        printf("\rProgress %0.2f ", progress); 
        fflush(stdout); d
    }
#endif
}