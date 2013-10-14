#ifndef WIN32
#error This project is only compatible with windows
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>
#include <pebble_app_info.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL_rotozoom.h>

#define RET_ERROR(msg,ret) {printf (msg); return ret;}

#define METADATA_FILE ".\\build\\local\\metadata.bin"
#define RESDEF_FILE ".\\build\\tempLocal\\winres.rc"
#define ICON_FILE ".\\build\\tempLocal\\icon.ico"
//#define ICON_TMP_PATH ".\\build\\tempLocal\\iconTemp\\"
#define ICON_TMP_FILE ".\\build\\tempLocal\\iconTemp\\%u"
//#define RESOURCE_PATH ".\\build\\local\\resources\\"
#define RESOURCE_FILE ".\\build\\local\\resources\\%u"

typedef struct __attribute__((__packed__)) {
    uint16_t pitch;
    uint16_t unknown;
    uint32_t type;
    uint16_t width;
    uint16_t height;
} GBitmapHeader;
typedef struct __attribute__((__packed__)) {
	uint16_t reserved; //must be 0
	uint16_t type; //must be 1
	uint16_t imagesCount;
} IconFileHeader;
typedef struct __attribute__((__packed__)) {
	uint8_t width; //0 for 256
	uint8_t height; //0 for 256
	uint8_t paletteSize; //0 for no-palette
	uint8_t reserved; //must be 0
	uint16_t colorPlanes; //must be 1
	uint16_t bitsPerPixel; //0 for end-of-list (see below)
	uint32_t dataSize;
	uint32_t dataOffset;
} IconElementHeader;
typedef struct __attribute__((__packed__)) {
	uint32_t headerSize; //must by 40
	uint32_t width;
	int32_t height; //must be negative for top->bottom pixel order
	uint16_t colorPlane; //must be 1
	uint16_t bitsPerPixel;
	uint32_t compression; //must be 0
	uint32_t dataSize;
	uint32_t horRes; //must be 0x0b13 TODO: Find better value?
	uint32_t verRes; //must be 0x0b13
	uint32_t paletteSize;
	uint32_t importantColors; //must be 0
} BmpDIBHeader;

#define ICON_ELEMENT(size,depth) {size,size,0,0,1,depth,0,0},
#define ICON_END_ELEMENT {0,0,0,0,0,0,0,0}
#define ICON_MAX_ELEMENTS 16
static IconElementHeader iconElementHeaders []={ //size only 16,24,32,48,64 or 128; depth only 8 or 32
	ICON_ELEMENT(16,8)
	ICON_ELEMENT(16,32)
	ICON_ELEMENT(32,8)
	ICON_ELEMENT(32,32)
	ICON_ELEMENT(64,32)
	ICON_END_ELEMENT
};

static PebbleAppInfo appInfo;

void dumpAppInfo ();
int extractIcon_loadPebbleImage (SDL_Surface** pblIconSurface);
int extractIcon_prepareIconElement (IconElementHeader* element,uint32_t baseOffset,SDL_Surface* pblIconSurface,uint32_t index);
int extractIcon ();
int generateResourceFile (char* projectName);

int main (int argc,char* argv[]) 
{
	int r;
	if (argc<2) RET_ERROR("usage: metaCompiler.exe <project_name> [-dump]\n",-1)
	if (SDL_Init(SDL_INIT_VIDEO)<0||IMG_Init(IMG_INIT_PNG)<0) RET_ERROR ("Couldn't initate SDL and SDL_image!",-2)
	FILE* f=fopen ("pebble_app.ld","rb");
    if (!f) RET_ERROR("Please execute the local simulator metadata compiler in the project main folder!\n",-3)
	fclose(f);
	f = fopen (METADATA_FILE,"rb");
	if (!f) RET_ERROR("Could not open metadata file!\n",-3)
	fread(&appInfo,1,sizeof(appInfo),f);
	fclose(f);
	CreateDirectoryA(".\\build\\tempLocal\\iconTemp\\",0);
	if (argc>2&&strcmp(argv[2],"-dump")==0)
		dumpAppInfo ();
	/*if (appInfo.icon_resource_id!=0) { //this functionality is broken! I will investigate more time in it later
		r=extractIcon ();
		if (r<0)
			return r;
	}*/
	r=generateResourceFile (argv[1]);
	IMG_Quit ();
	SDL_Quit ();
	return r;
}

int generateResourceFile (char* projectName)
{
	FILE* f=fopen (RESDEF_FILE,"w");
	if (!f) RET_ERROR("Couldn't open resource definition file!\n",-4)
	fprintf (f,
		"MAINICON ICON \".\\\\build\\\\tempLocal\\\\icon.ico\"\n"
		"1 VERSIONINFO\n"
		"FILEVERSION     %hhu,%hhu,0,0\n"
		"PRODUCTVERSION  %hhu,%hhu,0,0\n"
		"BEGIN\n"
		"\tBLOCK \"StringFileInfo\"\n"
		"\tBEGIN\n"
		"\t\tBLOCK \"040904E4\"\n"
		"\t\tBEGIN\n"
		"\t\t\tVALUE \"CompanyName\", \"%s\"\n"
		"\t\t\tVALUE \"FileDescription\", \"%s - Pebble Simulator by Helco\"\n"
		"\t\t\tVALUE \"FileVersion\", \"%hhu.%hhu\"\n"
		"\t\t\tVALUE \"InternalName\", \"%s\"\n"
		"\t\t\tVALUE \"LegalCopyright\", \"%s (c) %s\"\n"
		"\t\t\tVALUE \"OriginalFilename\", \"%s.exe\"\n"
		"\t\t\tVALUE \"ProductName\", \"%s\"\n"
		"\t\t\tVALUE \"ProductVersion\", \"%hhu.%hhu\"\n"
		"\t\tEND\n"
		"\tEND\n"
		"\tBLOCK \"VarFileInfo\"\n"
		"\tBEGIN\n"
		"\t\tVALUE \"Translation\", 0x409, 1252\n"
		"\tEND\n"
		"END\n",
		appInfo.app_version.major,appInfo.app_version.minor,appInfo.app_version.major,appInfo.app_version.minor,
		appInfo.company,appInfo.name,appInfo.app_version.major,appInfo.app_version.minor,projectName,appInfo.name,
		appInfo.company,projectName,appInfo.name,appInfo.app_version.major,appInfo.app_version.minor);
	fclose(f);
	return 0;
}

#define LOCK(X) if(SDL_MUSTLOCK(X)) SDL_LockSurface(X)
#define UNLOCK(X) if(SDL_MUSTLOCK(X)) SDL_UnlockSurface(X)
#define COLOR_BLACK 0x000000ff
#define COLOR_WHITE 0xffffffff
#define create32BitSurface(w,h) (SDL_CreateRGBSurface (SDL_SWSURFACE|SDL_SRCALPHA,w,h,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff))
#define CALL_RET_ERROR(call) r=(call);if (r<0) return r

int extractIcon ()
{
	SDL_Surface* pblIconSurface;
	int r;
	CALL_RET_ERROR(extractIcon_loadPebbleImage (&pblIconSurface));
	//Prepare icon file headers
	IconFileHeader header={0,1,0};
	for (header.imagesCount=0;header.imagesCount<ICON_MAX_ELEMENTS;header.imagesCount++) {
		if (iconElementHeaders[header.imagesCount].bitsPerPixel==0)
			break;
	}
	if (header.imagesCount==0) RET_ERROR ("Internal: No icon elements specified!\n",-9)
	uint32_t baseOffset=sizeof(IconFileHeader)+header.imagesCount*sizeof(IconElementHeader);
	for (uint32_t i=0;i<header.imagesCount;i++) {
		CALL_RET_ERROR(extractIcon_prepareIconElement (iconElementHeaders+i,baseOffset,pblIconSurface,i));
		baseOffset+=iconElementHeaders[i].dataSize;
	}
	SDL_FreeSurface (pblIconSurface);
	//write icon file
	uint8_t iconElementBuffer [1024];
	size_t len;
	FILE* f=fopen (ICON_FILE,"wb");
	if (!f) RET_ERROR("Couldn't open icon file!\n",-15)
	fwrite(&header,1,sizeof(header),f);
	fwrite(iconElementHeaders,header.imagesCount,sizeof(IconElementHeader),f);
	for (uint32_t i=0;i<header.imagesCount;i++) {
		char fileBuffer [64]={0};
		char* formatString=ICON_TMP_FILE;
		sprintf (fileBuffer,formatString,i);
		FILE* in =fopen (fileBuffer,"rb");
		if (!in) RET_ERROR("Couldn't reopen temporary icon file!\n",-16)
		do
		{
			len=fread(iconElementBuffer,1,1024,in);
			if (len>0)
				fwrite(iconElementBuffer,1,1024,f);
		} while (len==1024);
		fclose(in);
	}
	fclose(f);
	return 0;
}

int extractIcon_prepareIconElement (IconElementHeader* element,uint32_t baseOffset,SDL_Surface* pblIconSurface,uint32_t index) {
	//Convert surface 
	SDL_PixelFormat pixelFormat;
	if (element->bitsPerPixel==32)
		pixelFormat=(SDL_PixelFormat){0,32,4,0,0,0,0,24,16,8,0,0xff000000,0x00ff0000,0x0000ff00,0x000000ff,0,255};
	else {
		element->paletteSize=255;
		SDL_Palette* palette=(SDL_Palette*)malloc(sizeof(SDL_Palette));
		if (!palette) RET_ERROR("Couldn't allocate enough memory for palette structure!\n",-10)
		palette->ncolors=255;
		palette->colors=(SDL_Color*)malloc(element->paletteSize*sizeof(SDL_Color));
		if (!palette) RET_ERROR("Couldn't allocate enough memory for palette!\n",-11)
		for (uint16_t i=0;i<255;i++)
			palette->colors[i]=(SDL_Color){i,i,i,255};
		pixelFormat=(SDL_PixelFormat){palette,8,1,0,0,0,0,24,16,8,0,0xff000000,0x00ff0000,0x0000ff00,0x000000ff,0,255};
	}
	double zoom=(double)element->width/pblIconSurface->w;
	SDL_Surface* resized=zoomSurface(pblIconSurface,zoom,zoom,SMOOTHING_ON);
	if (!resized) RET_ERROR("Couldn't allocate enough memory for resized icon surface!\n",-12)
	if (element->width==32&&element->bitsPerPixel==32)
		SDL_SaveBMP(resized,"resized.bmp");
	SDL_Surface* converted=SDL_ConvertSurface(resized,&pixelFormat,SDL_SWSURFACE|SDL_SRCALPHA);
	if (!converted) RET_ERROR("Couldn't allocate enough memory for converted icon surface!\n",-13)
	if (element->width==32&&element->bitsPerPixel==32)
		SDL_SaveBMP(converted,"converted.bmp");
	SDL_FreeSurface(resized);
	if (element->width==32&&element->bitsPerPixel==32)
		SDL_SaveBMP(converted,"converted2.bmp");
	//Fill header information
	uint32_t pitch=(uint32_t)element->width*pixelFormat.BytesPerPixel;
	if (pitch%4>0)
		pitch+=4-(pitch%4);
	uint32_t size=40+element->paletteSize*4+pitch*element->width;
	element->dataSize=size;
	element->dataOffset=baseOffset;
	BmpDIBHeader dibHeader={40,element->width,element->width,1,element->bitsPerPixel,0,pitch*element->width,0x0b13,0x0b13,element->paletteSize,0};
	char fileBuffer [64]={0};
	char* formatString=ICON_TMP_FILE;
	sprintf (fileBuffer,formatString,index);
	//write temporary file
	FILE* f =fopen (fileBuffer,"wb");
	if (!f) RET_ERROR ("Couldn't open temporary icon element file!\n",-14)
	fwrite(&dibHeader,1,sizeof(dibHeader),f);
	if (element->paletteSize>0)
		fwrite (pixelFormat.palette->colors,element->paletteSize,4,f);
	LOCK(converted);
	fwrite(converted->pixels,1,pitch*element->width,f);
	UNLOCK(converted);
	fclose(f);
	//free allocated memory
	SDL_FreeSurface(converted);
	if (element->bitsPerPixel==8) {
		free (pixelFormat.palette->colors);
		free (pixelFormat.palette);
	}
	return 0;
}

int extractIcon_loadPebbleImage (SDL_Surface** pblIconSurface)
{
	//Load pebble image
	char pblIconFileBuffer [64]={0};
	char* pblFormatString=RESOURCE_FILE;
	GBitmapHeader pblIconHeader;
	sprintf (pblIconFileBuffer,pblFormatString,appInfo.icon_resource_id-1);
	FILE* f=fopen(pblIconFileBuffer,"rb");
	if (!f) RET_ERROR("Couldn't open icon resource file!\n",-5)
	fread (&pblIconHeader,1,sizeof(GBitmapHeader),f);
	uint8_t* pblIconBuffer=(uint8_t*)malloc(pblIconHeader.pitch*pblIconHeader.height);
	if (!pblIconBuffer) RET_ERROR("Couldnt allocate enough memory for pebble icon buffer!\n",-6)
	fread (pblIconBuffer,pblIconHeader.pitch*pblIconHeader.height,1,f);
	fclose(f);
	uint32_t pblIconFillColor;
	if ((*pblIconBuffer&1)>0) //the top left corner decides whether the icon should be filled with black or white
		pblIconFillColor=COLOR_WHITE;
	else
		pblIconFillColor=COLOR_BLACK;
	//copy pebble image data into a SDL_Surface
	SDL_Surface* pblIconRaw =create32BitSurface(pblIconHeader.width,pblIconHeader.height); //it will be rendered into a square surface later
	if (!pblIconRaw) RET_ERROR("Couldn't allocate enough memory for pebble icon surface!\n",-7)
	LOCK(pblIconRaw);
	uint32_t* pblIconScanline=(uint32_t*)pblIconRaw->pixels,* pblIconPixel;
	uint8_t* pblIconByte=pblIconBuffer,pblIconByteIndex,pblIconBitIndex=0;
	for (uint32_t y=0;y<pblIconHeader.height;y++) {
		pblIconPixel=pblIconScanline;
		pblIconByteIndex=0;
		pblIconBitIndex=0;
		for (uint32_t x=0;x<pblIconHeader.width;x++) {
			if ((pblIconByte[pblIconByteIndex]&(1<<pblIconBitIndex))>0)
				*pblIconPixel=COLOR_WHITE;
			else
				*pblIconPixel=COLOR_BLACK;
			pblIconPixel++;
			pblIconBitIndex++;
			if (pblIconBitIndex==8) {
				pblIconBitIndex=0;
				pblIconByteIndex++;
			}
		}
		pblIconByte+=pblIconHeader.pitch;
		pblIconScanline=(uint32_t*)((uint8_t*)pblIconScanline+pblIconRaw->pitch);
	}
	UNLOCK(pblIconRaw);
	free(pblIconBuffer);
	//Square the icon surface if needed
	if (pblIconHeader.width!=pblIconHeader.height) {
		uint32_t pblIconSize=pblIconHeader.width;
		if (pblIconHeader.height>pblIconSize)
			pblIconSize=pblIconHeader.height;
		*pblIconSurface=create32BitSurface(pblIconSize,pblIconSize);
		if (!pblIconSurface) RET_ERROR("Couldn't allocate enough memory for pebble icon square surface!\n",-8)
		SDL_FillRect(*pblIconSurface,0,pblIconFillColor);
		SDL_Rect dst={pblIconSize/2-pblIconHeader.width/2,pblIconSize/2-pblIconHeader.height/2,pblIconHeader.width,pblIconHeader.height};
		SDL_BlitSurface(pblIconRaw,0,*pblIconSurface,&dst);
		SDL_FreeSurface(pblIconRaw);
	}
	else
		*pblIconSurface=pblIconRaw;
	return 0;
}

void dumpAppInfo ()
{
	printf (
		"Header: \"%s\"\n"
		"Header version: %hhu.%hhu\n"
		"SDK version: %hhu.%hhu\n"
		"App version: %hhu.%hhu\n"
		"Size (invalid): %hu\n"
		"Offset (invalid): %u\n"
		"CRC (invalid): %u\n"
		"Name: \"%s\"\n"
		"Company: \"%s\"\n"
		"Icon resource ID: %u\n"
		"Symbol table address (invalid): %u\n"
		"Flags: ",
		appInfo.header,appInfo.struct_version.major,appInfo.struct_version.minor,appInfo.sdk_version.major,appInfo.sdk_version.minor,
		appInfo.app_version.major,appInfo.app_version.minor,appInfo.size,appInfo.offset,appInfo.crc,appInfo.name,appInfo.company,
		appInfo.icon_resource_id,appInfo.sym_table_addr);
	if ((appInfo.flags&APP_INFO_WATCH_FACE)>0)
		printf ("WATCH_FACE");
	else
		printf ("STANDARD_APP");
	if ((appInfo.flags&APP_INFO_VISIBILITY_HIDDEN)>0)
		printf (" | VISIBILITY_HIDDEN");
	if ((appInfo.flags&APP_INFO_VISIBILITY_SHOWN_ON_COMMUNICATION)>0)
		printf (" | VISIBILITY_SHOWN_ON_COMMUNICATION");
	printf ("\n"
		"Relocation list address (invalid): %u\n"
		"Relocation list entry count (invalid): %u\n"
		"UUID: %.2hhx%.2hhx%.2hhx%.2hhx-%.2hhx%.2hhx-%.2hhx%.2hhx-%.2hhx%.2hhx-%.2hhx%.2hhx%.2hhx%.2hhx%.2hhx%.2hhx\n",
		appInfo.reloc_list_start,appInfo.num_reloc_entries,appInfo.uuid.byte0,appInfo.uuid.byte1,appInfo.uuid.byte2,appInfo.uuid.byte3,
		appInfo.uuid.byte4,appInfo.uuid.byte5,appInfo.uuid.byte6,appInfo.uuid.byte7,appInfo.uuid.byte8,appInfo.uuid.byte9,
		appInfo.uuid.byte10,appInfo.uuid.byte11,appInfo.uuid.byte12,appInfo.uuid.byte13,appInfo.uuid.byte14,appInfo.uuid.byte15);
}
