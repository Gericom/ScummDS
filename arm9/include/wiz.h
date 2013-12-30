#ifndef __WIZ_H__
#define __WIZ_H__

struct WizImage {
	int resNum;
	int x1;
	int y1;
	int zorder;
	int state;
	int flags;
	int shadow;
	int field_390;
	int palette;
};

struct WizParameters {
	int field_0;
	byte filename[260];
	//Common::Rect box;
	int processFlags;
	int processMode;
	int field_11C;
	int field_120;
	int field_124;
	int field_128;
	int field_12C;
	int field_130;
	int field_134;
	int field_138;
	int compType;
	int fileWriteMode;
	int angle;
	int scale;
	int polygonId1;
	int polygonId2;
	int resDefImgW;
	int resDefImgH;
	int sourceImage;
	int params1;
	int params2;
	uint8 remapColor[256];
	uint8 remapIndex[256];
	int remapNum;
	int dstResNum;
	uint16 fillColor;
	byte string1[4096];
	byte string2[4096];
	int field_2399;
	int field_239D;
	int field_23A1;
	int field_23A5;
	int field_23A9;
	int field_23AD;
	int field_23B1;
	int field_23B5;
	int field_23B9;
	int field_23BD;
	int field_23C1;
	int field_23C5;
	int field_23C9;
	int field_23CD;
	//Common::Rect box2;
	int field_23DE;
	int spriteId;
	int spriteGroup;
	int field_23EA;
	WizImage img;
};

enum WizImageFlags {
	kWIFHasPalette = 0x1,
	kWIFRemapPalette = 0x2,
	kWIFPrint = 0x4,
	kWIFBlitToFrontVideoBuffer = 0x8,
	kWIFMarkBufferDirty = 0x10,
	kWIFBlitToMemBuffer = 0x20,
	kWIFIsPolygon = 0x40,
	kWIFFlipX = 0x400,
	kWIFFlipY = 0x800
};

enum WizProcessFlags {
	kWPFSetPos = 0x1,
	kWPFShadow = 0x4,
	kWPFScaled = 0x8,
	kWPFRotate = 0x10,
	kWPFNewFlags = 0x20,
	kWPFRemapPalette = 0x40,
	kWPFClipBox = 0x200,
	kWPFNewState = 0x400,
	kWPFUseFile = 0x800,
	kWPFUseDefImgWidth = 0x2000,
	kWPFUseDefImgHeight = 0x4000,
	kWPFPaletteNum = 0x8000,
	kWPFDstResNum = 0x10000,
	kWPFFillColor = 0x20000,
	kWPFClipBox2 = 0x40000,
	kWPFMaskImg = 0x80000,
	kWPFParams = 0x100000
}; 

struct AWIZ_t
{
	void* RGBS;
	uint32_t Compression;
	uint32_t Width;
	uint32_t Height;
	fpos_t WIZD;
};

extern WizParameters _wizParams;

void displayWizImage(WizImage *pwi);
void processWizImage(const WizParameters *params);
void loadWizCursor(int resId, int palette);

#endif