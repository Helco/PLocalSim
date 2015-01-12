//TODO: Add compability for windows/mac?
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "jsmn.h"
#ifdef WIN32
#include <windows.h>
#endif

#define VERSION_DEF_MAXLEN 63
#define FRIENDLY_VERSION_MAXLEN 31
#define RESOURCES_MAX_COUNT 32
#define RESOURCE_NAME_MAXLEN VERSION_DEF_MAXLEN
#define RESOURCE_FILE_MAXLEN 63
#define PNG_TRANS_POSTFIX_LEN 6

#define JSMN_TOKENS_MAX 256

#define RET_ERROR(msg,ret) {printf (msg); return ret;}

int resCount=0;
char resourceNames [RESOURCES_MAX_COUNT][RESOURCE_NAME_MAXLEN+1];
FILE* output=0;

int cmptoken (const char* buffer,jsmntok_t token,const char* string);
int compileResourceMap (FILE* f);
int generateResourceHeader ();

int main(int argc,char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ||
		IMG_Init(IMG_INIT_PNG) < 0)
		RET_ERROR ("Couldn't initate SDL and SDL_image!",-1)
    int i;
    FILE* f=fopen ("appinfo.json","rb");
    if (!f) RET_ERROR("Please execute the local simulator resource compiler in the project main folder!\n",-1)
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
    i=compileResourceMap (f);
    if (i!=0)
        return i;
    i=generateResourceHeader ();
    IMG_Quit ();
    SDL_Quit ();
    return i;
}

typedef int (*ResourceHandler) (char*,int);
int handleRawResource (char* file,int resIndex);
int handlePngResource (char* file,int resIndex);
int handlePngTransResource (char* file,int resIndex);
int handleFontResource (char* file,int resIndex);

int tokenTokenLength (jsmntok_t* tokens,int parent) {
    int len=1;
    if (tokens[parent].type==JSMN_ARRAY||tokens[parent].type==JSMN_OBJECT) {
        for (int i=0;i<tokens[parent].size;i++)
            len+=tokenTokenLength(tokens,parent+len);
    }
    return len;
}

int skipToToken (jsmntok_t* tokens,char* buffer,int parent,const char* name) {
    int tokenCursor=parent+1,i;
    if (tokens[parent].type!=JSMN_OBJECT)
        return -1;
    for (i=0;i<tokens[parent].size;i++) { //the last element doesn't have to be checked as there has to be another element following to be valid
        if (cmptoken(buffer,tokens[tokenCursor],name)>0)
            return (i==tokens[parent].size-1?-1:tokenCursor);
        tokenCursor+=tokenTokenLength(tokens,tokenCursor);
    }
    return -1;
}

int compileResourceMap (FILE* f) {
    char* buffer;
    jsmntok_t tokens [JSMN_TOKENS_MAX];
    jsmnerr_t err;
    jsmn_parser parser;
    size_t len;
    ResourceHandler resHandler;
    char resFileBuf [RESOURCE_FILE_MAXLEN+1];
    int parentToken,resourceToken,fragmentToken,r,resElement;
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
    parentToken=skipToToken(tokens,buffer,0,"resources")+1;
    if (parentToken<1||tokens[parentToken].type!=JSMN_OBJECT) RET_ERROR ("Resource map invalid (No object \"resource\" found)\n",-5)
    parentToken=skipToToken(tokens,buffer,parentToken,"media")+1;
    if (parentToken<1||tokens[parentToken].type!=JSMN_ARRAY) RET_ERROR ("Resource map invalid (No array \"media\" found)\n",-6)
    resourceToken=parentToken+1;
    for (resElement=0;resElement<tokens[parentToken].size;resElement++) {
        if (tokens[resourceToken].type!=JSMN_OBJECT) RET_ERROR ("Resource map invalid (Resource element is not an object)\n",-7)

        //read name
        fragmentToken=skipToToken(tokens,buffer,resourceToken,"name")+1;
        if (fragmentToken<1||tokens[fragmentToken].type!=JSMN_STRING) RET_ERROR("Resource map invalid (Resource string \"name\" not found)\n",-8)
        len=tokens[fragmentToken].end-tokens[fragmentToken].start;
        if (len>RESOURCE_NAME_MAXLEN) RET_ERROR("Resource map invalid (Resource name is too long)\n",-9)
        memcpy(resourceNames[resCount],buffer+tokens[fragmentToken].start,len);
        resourceNames[resCount][len]=0;

        //read file
        fragmentToken=skipToToken(tokens,buffer,resourceToken,"file")+1;
        if (fragmentToken<1||tokens[fragmentToken].type!=JSMN_STRING) RET_ERROR("Resource map invalid (Resource string \"file\" not found)\n",-10)
        len=tokens[fragmentToken].end-tokens[fragmentToken].start;
        if (len>RESOURCE_FILE_MAXLEN) RET_ERROR("Resource map invalid (Resource file is too long)\n",-11)
        memcpy(resFileBuf,buffer+tokens[fragmentToken].start,len);
        resFileBuf[len]=0;

        //read type
        resHandler=0;
        fragmentToken=skipToToken(tokens,buffer,resourceToken,"type")+1;
        if (fragmentToken<1||tokens[fragmentToken].type!=JSMN_STRING) RET_ERROR("Resource map invalid (Resource string \"type\" not found)\n",-11)
             if (cmptoken(buffer,tokens[fragmentToken],"raw")>0)        resHandler=handleRawResource;
        else if (cmptoken(buffer,tokens[fragmentToken],"png")>0)        resHandler=handlePngResource;
        else if (cmptoken(buffer,tokens[fragmentToken],"png-trans")>0) {
            len=strlen(resourceNames[resCount]);
            if (len+PNG_TRANS_POSTFIX_LEN>RESOURCE_FILE_MAXLEN) RET_ERROR("Resource map invalid (Resource name is too long (for a png-trans name))\n",-13)
            strcpy(resourceNames[resCount+1],resourceNames[resCount]);
            strcat(resourceNames[resCount],"_WHITE");
            strcat(resourceNames[resCount+1],"_BLACK");
            resCount++;
            resHandler=handlePngTransResource; //Verify if this exist
        }
        else if (cmptoken(buffer,tokens[fragmentToken],"font")>0)       resHandler=handleFontResource;
        else RET_ERROR("Resource map invalid (Invalid resource type)\n",-12)
        r=resHandler(resFileBuf,++resCount);
        if (r<0)
            return r;
        resourceToken+=tokenTokenLength(tokens,resourceToken);
    }
    free(buffer);
    return 0;
}

#define JSMN_BUFFER_SIZE 1024

int handleRawResource (char* fileBuf,int index) {
    char fileBuffer [RESOURCE_FILE_MAXLEN+32];
    sprintf (fileBuffer,"./resources/%s",fileBuf);
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

typedef struct __attribute__((__packed__)) {
    uint16_t pitch;
    uint16_t unknown; //default: 4096
    uint32_t type;//for png: 0 and for png-trans: 5
    uint16_t width;
    uint16_t height;
} RBitmap;

int handlePngResource (char* fileBuf,int index) {
    char fileBuffer [RESOURCE_FILE_MAXLEN+32];
    sprintf (fileBuffer,"./resources/%s",fileBuf);
    SDL_Surface* sur=IMG_Load (fileBuffer);
    if  (!sur) RET_ERROR ("Couldn't load png input file\n",-30)
		SDL_Surface* img = SDL_ConvertSurfaceFormat(sur, SDL_PIXELFORMAT_RGBA8888, 0);
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
    index--; //the given index points to the second image
    char fileBuffer [RESOURCE_FILE_MAXLEN+32];
    sprintf (fileBuffer,"./resources/%s",fileBuf);
    SDL_Surface* sur=IMG_Load (fileBuffer);
    if  (!sur) RET_ERROR ("Couldn't load png-trans input file\n",-34)
		SDL_Surface* img = SDL_ConvertSurfaceFormat(sur, SDL_PIXELFORMAT_RGBA8888, 0);
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
    sprintf (fileBuffer,"./resources/%s",fileBuf);
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

    char* sizePointer=resourceNames[index]+strlen(resourceNames[index-1])-1;
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

int cmptoken(const char* buffer,jsmntok_t token,const char* string) {

    int index=token.start;
    if (token.type!=JSMN_STRING)
        return 0;
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
        "#include <stdint.h>\n"
        "/*Because of some reasons (perhaps SDL?) changing the entry point creates crashes,\n"
        "but because pebble.h includes this file and all your pebble apps include pebble.h\n"
        "I change the name of your \"main\" function to \"pbl_main\"*/\n"
        "#undef main\n"
        "#define main pbl_main\n"
        "typedef enum {\n"
        "  INVALID_RESOURCE = 0,\n"
        "  DEFAULT_MENU_ICON = 0, // Friendly synonym for use in \'PBL_APP_INFO()\' calls\n";
    static const char* sectionResourceIDStart="  RESOURCE_ID_";
    static const char* sectionResourceIDEnd=",\n";
    static const char* sectionEnd="\n} ResourceId;\n";

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
    fputs (sectionEnd,f);
    fclose (f);
    return 0;
}
