//www.dragonwins.com/domains/getteched/bmp/bmpfileformat.htm
#pragma pack(push, 1)
struct BMP_FileHeader {
    char type1;
    char type2;
    U32 size;
    U16 reserved1;
    U16 reserved2;
    U32 offBits;
};

struct BMP_ImageHeader {
    U32 size;
    U32 width;
    U32 height;
    U16 planes;
    U16 bitCount;
    U32 compression;
    U32 sizeImage;
    U32 xPelsPerMeter;
    U32 yPelsPerMeter;
    U32 clrUsed;
    U32 clrImportant;
};

struct BMP_Header {
    BMP_FileHeader fileHeader;
    BMP_ImageHeader imageHeader;
};
#pragma pack(pop)

struct BMP_Image {
    BMP_Header header;
    U32* pixelData;
};
