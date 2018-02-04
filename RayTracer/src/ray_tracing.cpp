struct Internal_RayTraceResult {
    U32 hit;
    U32 hitMatIndex;
    V3 hitNormal;
    V3 hitPoint;
    
    //debug
    U32 hitId;
    char* hitName;
};

Internal void Internal_RayTrace(V3 rayOrigin, V3 rayDirection,
                                World* world,
                                F32 traceMaxDistance,
                                Internal_RayTraceResult* result) {
    F32 tolerance = 0.0001;
    
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
        
        if((divisor < tolerance) || (divisor > tolerance)) {
            F32 divident = Inner(currentPlane.p, currentPlane.n) - Inner(rayOrigin, currentPlane.n);
            F32 t = divident / divisor;
            
            if(t > tolerance &&  t < hitDistance) {
                hitDistance = t;
                hitMatIndex = currentPlane.matIndex;
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



V3 RayTrace(V3 rayOrigin, V3 rayDirection,
            World* world) {
    Material* materials = world->materials;
    
    Internal_RayTraceResult result = {};
    Internal_RayTrace(rayOrigin,
                      rayDirection,
                      world,
                      F32_MAX,
                      &result);
    
    V3 resultColor = {};
    if(result.hit) {
        Light* lights = world->lights;
        U32 lightCount = world->lightCount;
        
        F32 lightContribution = 1.0f / lightCount;
        V3 materialColor = materials[result.hitMatIndex].color;
        for(U32 lightIndex = 0; 
            lightIndex < lightCount;
            ++lightIndex) {
            Light currentLight = lights[lightIndex];
            
            F32 shadowBias = 0.0001f;
            V3 lightRayOrigin = result.hitPoint + result.hitNormal * shadowBias;
            
            V3 lightRayDirection = {};
            V3 lightIntensity = {1,1,1};
            F32 traceMaxDistance = F32_MAX;
            switch(currentLight.type) {
                case(LightType_Directional):  {
                    lightRayDirection = Normalize(currentLight.d.invertedDirection);
                    
                    F32 shading = Inner(result.hitNormal, Normalize(currentLight.d.invertedDirection));
                    shading= Max(shading, 0);
                    
                    lightIntensity = currentLight.color * currentLight.intensity * shading;
                } break;
                case(LightType_Point): {
                    V3 direction = currentLight.p.origin - lightRayOrigin;
                    F32 rSquare = Inner(direction);
                    lightRayDirection = Normalize(direction);
                    traceMaxDistance = SquareRoot(rSquare);
                    
                    V3 fallOff = (currentLight.color*currentLight.intensity) / (4.0f*PI*rSquare);
                    
                    F32 shading = Inner(result.hitNormal, Normalize(lightRayDirection));
                    shading = Max(shading, 0);
                    
                    lightIntensity = fallOff * shading;
                } break;
            }
            
            Internal_RayTraceResult lightResult = {};
            Internal_RayTrace(lightRayOrigin, lightRayDirection,
                              world,
                              traceMaxDistance,
                              &lightResult);
            
            bool selfIntersect = lightResult.hitId == result.hitId;
            F32 visible = (F32)(!lightResult.hit || (lightResult.hit && selfIntersect));
            
            
            V3 shading = (lightIntensity * visible);
            resultColor = resultColor + materialColor * shading * lightContribution;
        }
    } else {
        resultColor = materials[result.hitMatIndex].color;
    }
    
    return resultColor;
}