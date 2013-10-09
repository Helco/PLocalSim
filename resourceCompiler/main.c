//TODO: Add compability for windows/mac?
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "jsmn.h"
#ifdef WIN32
#include <windows.h>
#endif

#define VERSION_DEF_MAXLEN 63
#define FRIENDLY_VERSION_MAXLEN 31
#define RESOURCES_MAX_COUNT 32
#define RESOURCE_NAME_MAXLEN VERSION_DEF_MAXLEN
#define RESOURCE_FILE_MAXLEN 63

#define JSMN_TOKENS_MAX 256

#define RET_ERROR(msg,ret) {printf (msg); return ret;}

int resCount=0;
char friendlyVersion [FRIENDLY_VERSION_MAXLEN+1];
char versionDefName [VERSION_DEF_MAXLEN+1];
char resourceNames [RESOURCES_MAX_COUNT][RESOURCE_NAME_MAXLEN+1];
FILE* output=0;

int cmptoken (char* buffer,jsmntok_t token,char* string);
int compileResourceMap ();
int generateResourceHeader ();

int main(int argc,char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO)<0||IMG_Init(IMG_INIT_PNG)<0) RET_ERROR ("Couldn't initate SDL and SDL_image!",-1)
    int i;
    FILE* f=fopen ("pebble_app.ld","rb");
    if (!f) RET_ERROR("Please execute the local simulator resource compiler in the project main folder!\n",-1)
        fclose(f);
    memset (friendlyVersion,0,FRIENDLY_VERSION_MAXLEN+1);
    memset (versionDefName,0,VERSION_DEF_MAXLEN+1);
    for (i=0; i<RESOURCES_MAX_COUNT; i++)
        memset (resourceNames[i],0,RESOURCE_NAME_MAXLEN+1);
#ifdef WIN32
	//This will only work on a valid sample project... 
    CreateDirectoryA(".\\build\\local\\resources\\",0);
    CreateDirectoryA(".\\build\\tempLocal\\",0);
	CreateDirectoryA(".\\build\\tempLocal\\src\\",0);
#else
    system("mkdir -p ./build/local/resources/");
    system("mkdir -p ./build/tempLocal/src/");
#endif
    i=compileResourceMap ();
    if (i!=0)
        return i;
    i=generateResourceHeader ();
    IMG_Quit ();
    SDL_Quit ();
    return i;
}

typedef enum ResType {
    RES_TYPE_RAW=0,
    RES_TYPE_PNG,
    RES_TYPE_PNG_TRANS,
    RES_TYPE_FONT,
    RES_TYPE_MAX
} ResType;

int handleRawResource (char* file,int resIndex);
int handlePngResource (char* file,int resIndex);
int handlePngTransResource (char* file,int resIndex);
int handleFontResource (char* file,int resIndex);

int compileResourceMap () {
    char* buffer;
    jsmntok_t tokens [JSMN_TOKENS_MAX];
    jsmnerr_t err;
    jsmn_parser parser;
    size_t len;
    int i,j,k;
    ResType resType;
    char resFileBuf [RESOURCE_FILE_MAXLEN+1];
    int characterRegex; //only to check if it already was set
    int tokenCursor=0;
    FILE* f=fopen("./resources/src/resource_map.json","r");
    if (!f) RET_ERROR("Couldn't open resource_map.json!\n",-2)
        fseek(f,0,SEEK_END);
    len=ftell(f);
    fseek(f,0,SEEK_SET);
    buffer=(char*)malloc(len+1);
    if (!buffer) RET_ERROR("Couldn't allocate enough memory for map buffer",-3)
        fread(buffer,1,len,f);
    buffer[len]=0;
    fclose(f);

    jsmn_init (&parser);
    err=jsmn_parse (&parser,buffer,tokens,JSMN_TOKENS_MAX);
    if (err!=JSMN_SUCCESS) {
        char buffer[64]="";
        if (err==JSMN_ERROR_INVAL)
            strcat(buffer,"Resource map syntax is invalid(INVAL)\n");
        else if (err==JSMN_ERROR_NOMEM)
            strcat(buffer,"Resource map syntax is invalid(NOMEM)\n");
        else if (err==JSMN_ERROR_PART)
            strcat(buffer,"Resource map syntax is invalid(PART)\n");
        puts (buffer);
        return -3;
    }
    if (tokens[0].type!=JSMN_OBJECT) RET_ERROR ("Resource map invalid (First token is not an object)\n",-4)
        if (tokens[0].size!=6) RET_ERROR ("Resource map invalid (Main object has not enough or unknown elements)\n",-5)
            tokenCursor++;
    for (i=0; i<3; i++) {
        if (tokens[tokenCursor].type!=JSMN_STRING) RET_ERROR ("Resource map invalid (Main object element is not named)\n",-6)
            else if (cmptoken (buffer,tokens[tokenCursor],"friendlyVersion")) {
                tokenCursor++;
                if (tokens[tokenCursor].type!=JSMN_STRING) RET_ERROR("Resource map invalid (Friendly version value is not a string)\n",-7)
                    else if (tokens[tokenCursor].end-tokens[tokenCursor].start>FRIENDLY_VERSION_MAXLEN) RET_ERROR("Resource map invalid (Friendly version string is too long)\n",-8)
                        else if (friendlyVersion[0]!=0) RET_ERROR("Resource map invalid (Friendly version is set twice)\n",-9)
                            else {
                                memcpy(friendlyVersion,buffer+tokens[tokenCursor].start,tokens[tokenCursor].end-tokens[tokenCursor].start);
                                tokenCursor++;
                            }
            } else if (cmptoken (buffer,tokens[tokenCursor],"versionDefName")) {
                tokenCursor++;
                if (tokens[tokenCursor].type!=JSMN_STRING) RET_ERROR ("Resource map invalid (Version definition name is not a string)\n",-9)
                    else if (tokens[tokenCursor].end-tokens[tokenCursor].start>VERSION_DEF_MAXLEN) RET_ERROR("Resource map invalid (Version definition name is too long)\n",-10)
                        else if (versionDefName[0]!=0) RET_ERROR ("Resource map invalid (Version definition name is set twice)\n",-11)
                            else {
                                memcpy(versionDefName,buffer+tokens[tokenCursor].start,tokens[tokenCursor].end-tokens[tokenCursor].start);
                                tokenCursor++;
                            }
            } else if (cmptoken (buffer,tokens[tokenCursor],"media")) {
                tokenCursor++;
                if (tokens[tokenCursor].type!=JSMN_ARRAY) RET_ERROR ("Resource map invalid (Media element is not an array)\n",-12)
                    else {
                        resCount=tokens[tokenCursor].size;
                        tokenCursor++;
                        for (j=0; j<resCount; j++) {
                            if (tokens[tokenCursor].type!=JSMN_OBJECT) RET_ERROR ("Resource map invalid (Resource is not an object)\n",-13)
                                else {
                                    len=tokens[tokenCursor].size/2;
                                    tokenCursor++;
                                    resType=RES_TYPE_MAX;
                                    memset(resFileBuf,0,RESOURCE_FILE_MAXLEN+1);
                                    characterRegex=0;
                                    for (k=0; k<len; k++) {
                                        if (tokens[tokenCursor].type!=JSMN_STRING) RET_ERROR ("Resource map invalid (Resource element is not named)\n",-14)
                                            else if (cmptoken(buffer,tokens[tokenCursor],"type")) {
                                                tokenCursor++;
                                                if (tokens[tokenCursor].type!=JSMN_STRING) RET_ERROR ("Resource map invalid (Resource type is not a string)\n",-15)
                                                    else if (resType!=RES_TYPE_MAX) RET_ERROR ("Resource map invalid (Resource type is set twice)\n",-16)
                                                        else {
                                                            if (cmptoken(buffer,tokens[tokenCursor],"raw")) resType=RES_TYPE_RAW;
                                                            else if (cmptoken(buffer,tokens[tokenCursor],"png")) resType=RES_TYPE_PNG;
                                                            else if (cmptoken(buffer,tokens[tokenCursor],"png-trans")) {
                                                                resType=RES_TYPE_PNG_TRANS;
                                                                resCount++;
                                                                j++;
                                                            } else if (cmptoken(buffer,tokens[tokenCursor],"font")) resType=RES_TYPE_FONT;
                                                            else RET_ERROR("Resource map invalid (Unknown resource type)\n",-17)
                                                                tokenCursor++;
                                                        }
                                            } else if (cmptoken(buffer,tokens[tokenCursor],"file")) {
                                                tokenCursor++;
                                                if (tokens[tokenCursor].type!=JSMN_STRING) RET_ERROR ("Resource map invalid (Resource file is not a string)\n",-18)
                                                    else if (resFileBuf[0]!=0) RET_ERROR ("Resource map invalid (Resource file is set twice)\n",-19)
                                                        else if (tokens[tokenCursor].end-tokens[tokenCursor].start>RESOURCE_FILE_MAXLEN) RET_ERROR ("Resource map invalid (Resource file string is too long)\n",-20)
                                                            else {
                                                                memcpy(resFileBuf,buffer+tokens[tokenCursor].start,tokens[tokenCursor].end-tokens[tokenCursor].start);
                                                                tokenCursor++;
                                                            }
                                            } else if (cmptoken(buffer,tokens[tokenCursor],"defName")) {
                                                tokenCursor++;
                                                if (tokens[tokenCursor].type!=JSMN_STRING) RET_ERROR ("Resource map invalid (Resource definition name is not a string)\n",-21)
                                                    else if (resourceNames[j][0]!=0) RET_ERROR ("Resource map invalid (Resource definition name is set twice)\n",-22)
                                                        else if (tokens[tokenCursor].end-tokens[tokenCursor].start>RESOURCE_NAME_MAXLEN) RET_ERROR ("Resource map invalid (Resource definition name is too long)\n",-23)
                                                            else {
                                                                memcpy(resourceNames[j],buffer+tokens[tokenCursor].start,tokens[tokenCursor].end-tokens[tokenCursor].start);
                                                                tokenCursor++;
                                                            }
                                            } else if (cmptoken(buffer,tokens[tokenCursor],"characterRegex")) {
                                                tokenCursor++;
                                                if (tokens[tokenCursor].type!=JSMN_STRING) RET_ERROR ("Resource map invalid (Resource character regex is not a string)\n",-24)
                                                    else if (characterRegex!=0) RET_ERROR ("Resource map invalid (Resource character regex is set twice)\n",-25)
                                                        else {
                                                            characterRegex=1;
                                                            tokenCursor++;
                                                        }
                                            } else RET_ERROR("Resource map invalid (Unknown resource element)\n",-26)
                                            } //for every element in a resource object
                                    //Because of all these "...is set twice" checks I can now ensure that all informations are there
                                    if (resType==RES_TYPE_RAW)
                                        k=handleRawResource (resFileBuf,j);
                                    else if (resType==RES_TYPE_PNG)
                                        k=handlePngResource (resFileBuf,j);
                                    else if (resType==RES_TYPE_FONT)
                                        k=handleFontResource (resFileBuf,j);
                                    else { /*if (resType==RES_TYPE_PNG_TRANS*/
                                        if (resourceNames [j]==0)
                                            strcpy (resourceNames[j],resourceNames[j-1]);
                                        else
                                            strcpy (resourceNames[j-1],resourceNames[j]);
                                        strcat(resourceNames[j-1],"_WHITE");
                                        strcat(resourceNames[j],"_BLACK");
                                        k=handlePngTransResource (resFileBuf,j-1);
                                    }
                                    if (k!=0)
                                        return k;
                                }
                        }
                    }
            } else RET_ERROR ("Resource map invalid (Unknown main object element)\n",-27)
            }
    free(buffer);
    return 0;
}

#define JSMN_BUFFER_SIZE 1024

int handleRawResource (char* fileBuf,int index) {
    char fileBuffer [RESOURCE_FILE_MAXLEN+32];
    sprintf (fileBuffer,"./resources/src/%s",fileBuf);
    FILE* in = fopen (fileBuffer,"rb");
    if (!in) RET_ERROR("Couldn't open raw resource input file\n",-28)

        sprintf (fileBuffer,"./build/local/resources/%d",index);
    FILE* out= fopen (fileBuffer,"wb");
    if (!out) RET_ERROR("Couldn't open raw resource output file\n",-29)

        char buffer [JSMN_BUFFER_SIZE];
    size_t len = fread (buffer,1,JSMN_BUFFER_SIZE,in);
    while (len>0) {
        fwrite(buffer,1,len,out);
        len=fread(buffer,1,JSMN_BUFFER_SIZE,in);
    }
    fclose (in);
    fclose (out);
    return 0;
}

#define LOCK(X) if(SDL_MUSTLOCK(X)) SDL_LockSurface(X)
#define UNLOCK(X) if(SDL_MUSTLOCK(X)) SDL_UnlockSurface(X)

static SDL_PixelFormat pixelFormat= {
    .palette=0,
    .BitsPerPixel=32,
    .BytesPerPixel=32,
    .Rloss=0,.Gloss=0,.Bloss=0,.Aloss=0,
    .Rshift=24,.Gshift=16,.Bshift=8,.Ashift=0,
    .Rmask=0xff000000,.Gmask=0x00ff0000,.Bmask=0x0000ff00,.Amask=0x000000ff,
    .colorkey=0,
    .alpha=255
};

typedef struct __attribute__((__packed__)) {
    uint16_t pitch;
    uint16_t unknown; //default: 4096
    uint32_t type;//actually default but for png: 0 and for png-trans: 5
    uint16_t width;
    uint16_t height;
} RBitmap;

int handlePngResource (char* fileBuf,int index) {
    char fileBuffer [RESOURCE_FILE_MAXLEN+32];
    sprintf (fileBuffer,"./resources/src/%s",fileBuf);
    SDL_Surface* sur=IMG_Load (fileBuffer);
    if  (!sur) RET_ERROR ("Couldn't load png input file\n",-30)
        SDL_Surface* img=SDL_ConvertSurface (sur,&pixelFormat,SDL_SWSURFACE|SDL_SRCALPHA);
    if (!img) RET_ERROR ("Couldn't convert surface\n",-31)
        SDL_FreeSurface(sur);

    RBitmap result= {
        .pitch=4,
        .unknown=4096,
        .type=0,
        .width=img->w,
        .height=img->h
    };
    uint16_t byteIndex;
    uint8_t bitShift;
    byteIndex=img->w/8+(img->w%8>0);
    while (result.pitch<byteIndex)
        result.pitch<<=1;
    uint8_t* data=(uint8_t*)malloc(result.height*result.pitch);
    if (data==0) RET_ERROR ("Couldn't allocate enough memory for resource bitmap\n",-32)
        memset(data,0,result.height*result.pitch);
    uint32_t* pixel;
    uint16_t x,y;
    uint8_t r,g,b,a;

    LOCK(img);
    for (y=0; y<img->h; y++) {
        for (x=0; x<img->w; x++) {
            pixel=(uint32_t*)(((uint8_t*)img->pixels)+y*img->pitch+x*4);
            byteIndex=y*result.pitch+x/8;
            bitShift=x%8;
            SDL_GetRGBA(*pixel,img->format,(Uint8*)&r,(Uint8*)&g,(Uint8*)&b,(Uint8*)&a);
            if (a>0x7f&&((uint16_t)r+g+b)/3>0x7f)
                data[byteIndex]|=1<<bitShift;
        }
    }
    UNLOCK(img);
    SDL_FreeSurface (img);

    sprintf (fileBuffer,"./build/local/resources/%d",index);
    FILE* f=fopen (fileBuffer,"wb");
    if (!f) RET_ERROR ("Couldn't open output bitmap file\n",-33)
        fwrite(&result,1,sizeof(result),f);
    fwrite(data,1,result.height*result.pitch,f);
    fclose(f);
    free(data);
    return 0;
}

int handlePngTransResource (char* fileBuf,int index) {
    char fileBuffer [RESOURCE_FILE_MAXLEN+32];
    sprintf (fileBuffer,"./resources/src/%s",fileBuf);
    SDL_Surface* sur=IMG_Load (fileBuffer);
    if  (!sur) RET_ERROR ("Couldn't load png-trans input file\n",-34)
        SDL_Surface* img=SDL_ConvertSurface (sur,&pixelFormat,SDL_SWSURFACE|SDL_SRCALPHA);
    if (!img) RET_ERROR ("Couldn't convert surface\n",-35)
        SDL_FreeSurface(sur);

    RBitmap white= {
        .pitch=4,
        .unknown=4096,
        .type=5,
        .width=img->w,
        .height=img->h
    };
    uint16_t byteIndex;
    uint8_t bitShift;
    byteIndex=img->w/8+(img->w%8>0);
    while (white.pitch<byteIndex)
        white.pitch<<=1;
    uint8_t* dataWhite=(uint8_t*)malloc(white.height*white.pitch);
    if (dataWhite==0) RET_ERROR ("Couldn't allocate enough memory for resource white bitmap\n",-36)
        memset(dataWhite,0,white.height*white.pitch);
    uint8_t* dataBlack=(uint8_t*)malloc(white.height*white.pitch);
    if (dataBlack==0) RET_ERROR ("Couldn't allocate enough memory for resource black bitmap\n",-37)
        memset(dataBlack,0,white.height*white.pitch);
    uint32_t* pixel;
    uint16_t x,y;
    uint8_t r,g,b,a;

    LOCK(img);
    for (y=0; y<img->h; y++) {
        for (x=0; x<img->w; x++) {
            pixel=(uint32_t*)(((uint8_t*)img->pixels)+y*img->pitch+x*4);
            byteIndex=y*white.pitch+x/8;
            bitShift=x%8;
            SDL_GetRGBA(*pixel,img->format,(Uint8*)&r,(Uint8*)&g,(Uint8*)&b,(Uint8*)&a);
            if (a>0x7f) {
                if (((uint16_t)r+g+b)/3>0x7f)
                    dataWhite[byteIndex]|=1<<bitShift;
                else
                    dataBlack[byteIndex]|=1<<bitShift;
            }
        }
    }
    UNLOCK(img);
    SDL_FreeSurface (img);

    sprintf (fileBuffer,"./build/local/resources/%d",index);
    FILE* f=fopen (fileBuffer,"wb");
    if (!f) RET_ERROR ("Couldn't open output white bitmap file\n",-38)
        fwrite(&white,1,sizeof(white),f);
    fwrite(dataWhite,1,white.height*white.pitch,f);
    fclose(f);
    free(dataWhite);

    sprintf (fileBuffer,"./build/local/resources/%d",index+1);
    f=fopen (fileBuffer,"wb");
    if (!f) RET_ERROR ("Couldn't open output black bitmap file\n",-39)
        fwrite(&white,1,sizeof(white),f);
    fwrite(dataBlack,1,white.height*white.pitch,f);
    fclose(f);
    free(dataBlack);
    return 0;
}

int handleFontResource (char* fileBuf,int index) {
    char fileBuffer [RESOURCE_FILE_MAXLEN+32];
    sprintf (fileBuffer,"./resources/src/%s",fileBuf);
    FILE* in = fopen (fileBuffer,"rb");
    if (!in) RET_ERROR("Couldn't open font resource input file\n",-40)

        sprintf (fileBuffer,"./build/local/resources/%d_f",index);
    FILE* out= fopen (fileBuffer,"wb");
    if (!out) RET_ERROR("Couldn't open font resource output file\n",-41)

        char buffer [JSMN_BUFFER_SIZE];
    size_t len = fread (buffer,1,JSMN_BUFFER_SIZE,in);
    while (len>0) {
        fwrite(buffer,1,len,out);
        len=fread(buffer,1,JSMN_BUFFER_SIZE,in);
    }
    fclose (in);
    fclose (out);

    char* sizePointer=resourceNames[index]+strlen(resourceNames[index])-1;
    while (sizePointer!=resourceNames[index]&&*sizePointer>='0'&&*sizePointer<='9')
        sizePointer--;
    if (sizePointer==resourceNames[index]+strlen(resourceNames[index])-1||sizePointer==resourceNames[index]) RET_ERROR ("Font definition name is invalid (Size has to stated at the end)\n",-42)
        sizePointer++;
    int height=atoi(sizePointer);
    sprintf (fileBuffer,"./build/local/resources/%d",index);
    out=fopen (fileBuffer,"wb");
    fwrite(&height,1,sizeof(int),out);
    fclose(out);
    return 0;
}

int cmptoken(char* buffer,jsmntok_t token,char* string) {
    int index=token.start;
    while (index<token.end&&
            *string!=0&&
            *string==buffer[index]) {
        index++;
        string++;
    }
    return index>=token.end;
}

int generateResourceHeader() {
    static const char* fileName="./build/tempLocal/src/resource_ids.auto.h";
    static const char* sectionStart=
        "#pragma once\n"
        "\n"
        "//\n"
        "// AUTOGENERATED BY tools/local/resCompiler\n"
        "// DO NOT MODIFY\n"
        "//\n"
        "\n"
        "#include \"pebble_os.h\"\n"
        "typedef enum {\n"
        "  INVALID_RESOURCE = 0,\n"
        "  DEFAULT_MENU_ICON = 0, // Friendly synonym for use in \'PBL_APP_INFO()\' calls\n";
    static const char* sectionResourceIDStart="  RESOURCE_ID_";
    static const char* sectionResourceIDEnd=",\n";
    static const char* sectionResBankVersion=
        "\n} ResourceId;\n"
        "static const ResBankVersion ";
    static const char* sectionFriendlyVersion=
        " = {\n"
        "  .crc = 0,\n"
        "  .timestamp = 0,\n"
        "  .friendly_version = \"";
    static const char* sectionCrcTable=
        "\"\n"
        "};\n"
        "\n"
        "static const uint32_t resource_crc_table[] = {\n";
    static const char* sectionCrcElementStart="  0";
    static const char* sectionCrcElementEnd=",\n";
    static const char* sectionEnd="\n};\n";

    FILE* f=fopen (fileName,"w");
    int i;
    if (!f) RET_ERROR ("Couldn't open header file\n",-40)
        fputs (sectionStart,f);
    for (i=0; i<resCount; i++) {
        fputs (sectionResourceIDStart,f);
        fputs (resourceNames[i],f);
        if (i+1!=resCount)
            fputs (sectionResourceIDEnd,f);
    }
    fputs (sectionResBankVersion,f);
    fputs (versionDefName,f);
    fputs (sectionFriendlyVersion,f);
    fputs (friendlyVersion,f);
    fputs (sectionCrcTable,f);
    for (i=0; i<resCount; i++) {
        fputs (sectionCrcElementStart,f);
        if (i+1!=resCount)
            fputs(sectionCrcElementEnd,f);
    }
    fputs (sectionEnd,f);
    fclose (f);
    return 0;
}
