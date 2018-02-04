Internal U8 LinearToRGB(F32 v) {
    U8 result;
    
    if(v > 1.0f) {
        v = 1;
    }
    
    if(v < 0.0f) {
        v = 0.0f;
    }
    
    if(v <= 0.0031308f) {
        result = (U8)(v * 12.92f);
    } else {
        result = (U8)(1.055f * Pow(v, 1.0f / 2.4f) - 0.055f);
    }
    
    return result;
}

Internal inline F32 ClampColorComponent(F32 c) {
    F32 result = c;
    
    if(c > 1.0f) {
        result = 1.0f;
    }
    else if(c < 0.0f) {
        result = 0.0f;
    }
    
    return result;
}

Internal inline V3 ClampColor(V3 color) {
    V3 result;
    
    result.r = ClampColorComponent(color.r);
    result.g = ClampColorComponent(color.g);
    result.b = ClampColorComponent(color.b);
    
    return result;
}

Internal U32 PackColor(V3 color) {
    color = ClampColor(color);
    
    U8 r = (U8)(color.r * 255);
    U8 g = (U8)(color.g * 255);
    U8 b = (U8)(color.b * 255);
    
    
    U32 result = 0;
    
    //alpa
    result |= 255 << 24;
    
    //red
    result |= r << 16;
    
    //green
    result |= g << 8;
    
    //blue
    result |= b;
    
    return result;
}

Internal void InitBMPImage(BMP_Image* image, U32 width, U32 height) {
    U32 pixelCount = width * height;
    U32 pixelDataSize = pixelCount * sizeof(U32);
    
    BMP_FileHeader fileHeader = {};
    fileHeader.type1 = 'B';
    fileHeader.type2 = 'M';
    fileHeader.size = pixelDataSize + sizeof(BMP_FileHeader) + sizeof(BMP_ImageHeader);
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.offBits = sizeof(BMP_FileHeader) + sizeof(BMP_ImageHeader);
    image->header.fileHeader = fileHeader;
    
    BMP_ImageHeader imageHeader = {};
    imageHeader.size = sizeof(BMP_ImageHeader);
    imageHeader.width = width;
    imageHeader.height = height;
    imageHeader.planes = 1;
    imageHeader.bitCount = 32;
    imageHeader.compression = 0;
    imageHeader.sizeImage = pixelDataSize;
    imageHeader.xPelsPerMeter = 0;
    imageHeader.yPelsPerMeter = 0;
    imageHeader.clrUsed = 0;
    imageHeader.clrImportant = 0;
    image->header.imageHeader = imageHeader;
    
    U32* pixelData = (U32*)malloc(pixelDataSize);
    image->pixelData = pixelData;
}

Internal U32 GetPixelCount(BMP_Image* image) {
    return image->header.imageHeader.width * image->header.imageHeader.height;
}

Internal void WriteBMPImage(BMP_Image* image, char* fileName) {
    FILE* file = fopen(fileName, "wb");
    if(!file) {
        fprintf(stderr, "Not able to open result file for writing . . .");
        
        return;
    }
    
    fwrite((void*)&image->header,
           sizeof(BMP_Header),
           1,
           file);
    
    fwrite((void*)image->pixelData,
           image->header.imageHeader.sizeImage,
           1,
           file);
    
    fclose(file);
}

Internal void GetDimensions(BMP_Image* image,
                            U32* height,
                            U32* width) {
    *height = image->header.imageHeader.height; 
    *width = image->header.imageHeader.width;
}

Internal U32* GetPackedPixelData(BMP_Image* image) {
    return image->pixelData;
}