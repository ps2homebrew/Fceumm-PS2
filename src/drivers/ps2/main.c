#include <stdio.h>
#include <fileio.h>
#include <string.h>
#include <libjpg.h>

// FCEUltra headers
#include "../../driver.h"
#include "../../fceu-types.h"

#include "ps2fceu.h"
extern char path[4096];
extern vars Settings;
extern skin FCEUSkin;
char mpartitions[4][256];
int FONT_HEIGHT = 16;
int FONT_WIDTH = 8;
/************************************/
/* FCEUltra Variables               */
/************************************/
#ifdef SOUND_ON
  #include <audsrv.h>
#endif
FCEUGI *CurGame = NULL;
const char * GetKeyboard() {
    return "";
}
int PPUViewScanline = 0;
int PPUViewer = 0;
int UpdatePPUView = 0;

//extern struct st_palettes palettes[];
struct st_palettes {
    char name[32];
    char desc[32];
    unsigned int data[64];
};

// Palettes are in normal order
struct st_palettes palettes[] = {
    { "asqrealc", "AspiringSquire's Real palette",
      { 0x6c6c6c, 0x00268e, 0x0000a8, 0x400094,
        0x700070, 0x780040, 0x700000, 0x621600,
        0x442400, 0x343400, 0x005000, 0x004444,
        0x004060, 0x000000, 0x101010, 0x101010,
        0xbababa, 0x205cdc, 0x3838ff, 0x8020f0,
        0xc000c0, 0xd01474, 0xd02020, 0xac4014,
        0x7c5400, 0x586400, 0x008800, 0x007468,
        0x00749c, 0x202020, 0x101010, 0x101010,
        0xffffff, 0x4ca0ff, 0x8888ff, 0xc06cff,
        0xff50ff, 0xff64b8, 0xff7878, 0xff9638,
        0xdbab00, 0xa2ca20, 0x4adc4a, 0x2ccca4,
        0x1cc2ea, 0x585858, 0x101010, 0x101010,
        0xffffff, 0xb0d4ff, 0xc4c4ff, 0xe8b8ff,
        0xffb0ff, 0xffb8e8, 0xffc4c4, 0xffd4a8,
        0xffe890, 0xf0f4a4, 0xc0ffc0, 0xacf4f0,
        0xa0e8ff, 0xc2c2c2, 0x202020, 0x101010 } },
    { "nintendo-vc", "Wii Virtual Console",
      { 0x494949, 0x00006a, 0x090063, 0x290059,
        0x42004a, 0x490000, 0x420000, 0x291100,
        0x182700, 0x003010, 0x003000, 0x002910,
        0x012043, 0x000000, 0x000000, 0x000000,
        0x747174, 0x003084, 0x3101ac, 0x4b0194,
        0x64007b, 0x6b0039, 0x6b2101, 0x5a2f00,
        0x424900, 0x185901, 0x105901, 0x015932,
        0x01495a, 0x101010, 0x000000, 0x000000,
        0xadadad, 0x4a71b6, 0x6458d5, 0x8450e6,
        0xa451ad, 0xad4984, 0xb5624a, 0x947132,
        0x7b722a, 0x5a8601, 0x388e31, 0x318e5a,
        0x398e8d, 0x383838, 0x000000, 0x000000,
        0xb6b6b6, 0x8c9db5, 0x8d8eae, 0x9c8ebc,
        0xa687bc, 0xad8d9d, 0xae968c, 0x9c8f7c,
        0x9c9e72, 0x94a67c, 0x84a77b, 0x7c9d84,
        0x73968d, 0xdedede, 0x000000, 0x000000 } },
    { "rgb", "Nintendo RGB PPU (PC-10)",
      { 0x6D6D6D, 0x002492, 0x0000DB, 0x6D49DB,
        0x92006D, 0xB6006D, 0xB62400, 0x924900,
        0x6D4900, 0x244900, 0x006D24, 0x009200,
        0x004949, 0x000000, 0x000000, 0x000000,
        0xB6B6B6, 0x006DDB, 0x0049FF, 0x9200FF,
        0xB600FF, 0xFF0092, 0xFF0000, 0xDB6D00,
        0x926D00, 0x249200, 0x009200, 0x00B66D,
        0x009292, 0x242424, 0x000000, 0x000000,
        0xFFFFFF, 0x6DB6FF, 0x9292FF, 0xDB6DFF,
        0xFF00FF, 0xFF6DFF, 0xFF9200, 0xFFB600,
        0xDBDB00, 0x6DDB00, 0x00FF00, 0x49FFDB,
        0x00FFFF, 0x494949, 0x000000, 0x000000,
        0xFFFFFF, 0xB6DBFF, 0xDBB6FF, 0xFFB6FF,
        0xFF92FF, 0xFFB6B6, 0xFFDB92, 0xFFFF49,
        0xFFFF6D, 0xB6FF49, 0x92FF6D, 0x49FFDB,
        0x92DBFF, 0x929292, 0x000000, 0x000000 } },
    { "sony-cxa2025as-us", "Sony CXA2025AS US",
      { 0x585858, 0x00238C, 0x00139B, 0x2D0585,
        0x5D0052, 0x7A0017, 0x7A0800, 0x5F1800,
        0x352A00, 0x093900, 0x003F00, 0x003C22,
        0x00325D, 0x000000, 0x000000, 0x000000,
        0xA1A1A1, 0x0053EE, 0x153CFE, 0x6028E4,
        0xA91D98, 0xD41E41, 0xD22C00, 0xAA4400,
        0x6C5E00, 0x2D7300, 0x007D06, 0x007852,
        0x0069A9, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x1FA5FE, 0x5E89FE, 0xB572FE,
        0xFE65F6, 0xFE6790, 0xFE773C, 0xFE9308,
        0xC4B200, 0x79CA10, 0x3AD54A, 0x11D1A4,
        0x06BFFE, 0x424242, 0x000000, 0x000000,
        0xFFFFFF, 0xA0D9FE, 0xBDCCFE, 0xE1C2FE,
        0xFEBCFB, 0xFEBDD0, 0xFEC5A9, 0xFED18E,
        0xE9DE86, 0xC7E992, 0xA8EEB0, 0x95ECD9,
        0x91E4FE, 0xACACAC, 0x000000, 0x000000 } },
    { "pal", "PAL",
      { 0x808080, 0x0000BA, 0x3700BF, 0x8400A6,
        0xBB006A, 0xB7001E, 0xB30000, 0x912600,
        0x7B2B00, 0x003E00, 0x00480D, 0x003C22,
        0x002F66, 0x000000, 0x050505, 0x050505,
        0xC8C8C8, 0x0059FF, 0x443CFF, 0xB733CC,
        0xFE33AA, 0xFE375E, 0xFE371A, 0xD54B00,
        0xC46200, 0x3C7B00, 0x1D8415, 0x009566,
        0x0084C4, 0x111111, 0x090909, 0x090909,
        0xFEFEFE, 0x0095FF, 0x6F84FF, 0xD56FFF,
        0xFE77CC, 0xFE6F99, 0xFE7B59, 0xFE915F,
        0xFEA233, 0xA6BF00, 0x51D96A, 0x4DD5AE,
        0x00D9FF, 0x666666, 0x0D0D0D, 0x0D0D0D,
        0xFEFEFE, 0x84BFFF, 0xBBBBFF, 0xD0BBFF,
        0xFEBFEA, 0xFEBFCC, 0xFEC4B7, 0xFECCAE,
        0xFED9A2, 0xCCE199, 0xAEEEB7, 0xAAF8EE,
        0xB3EEFF, 0xDDDDDD, 0x111111, 0x111111 } },
    { "bmf-final2", "BMF's Final 2 palette",
      { 0x525252, 0x000080, 0x08008A, 0x2C007E,
        0x4A004E, 0x500006, 0x440000, 0x260800,
        0x0A2000, 0x002E00, 0x003200, 0x00260A,
        0x001C48, 0x000000, 0x000000, 0x000000,
        0xA4A4A4, 0x0038CE, 0x3416EC, 0x5E04DC,
        0x8C00B0, 0x9A004C, 0x901800, 0x703600,
        0x4C5400, 0x0E6C00, 0x007400, 0x006C2C,
        0x005E84, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x4C9CFF, 0x7C78FF, 0xA664FF,
        0xDA5AFF, 0xF054C0, 0xF06A56, 0xD68610,
        0xBAA400, 0x76C000, 0x46CC1A, 0x2EC866,
        0x34C2BE, 0x3A3A3A, 0x000000, 0x000000,
        0xFFFFFF, 0xB6DAFF, 0xC8CAFF, 0xDAC2FF,
        0xF0BEFF, 0xFCBCEE, 0xFAC2C0, 0xF2CCA2,
        0xE6DA92, 0xCCE68E, 0xB8EEA2, 0xAEEABE,
        0xAEE8E2, 0xB0B0B0, 0x000000, 0x000000 } },
    { "bmf-final3", "BMF's Final 3 palette",
      { 0x686868, 0x001299, 0x1A08AA, 0x51029A,
        0x7E0069, 0x8E001C, 0x7E0301, 0x511800,
        0x1F3700, 0x014E00, 0x005A00, 0x00501C,
        0x004061, 0x000000, 0x000000, 0x000000,
        0xB9B9B9, 0x0C5CD7, 0x5035F0, 0x8919E0,
        0xBB0CB3, 0xCE0C61, 0xC02B0E, 0x954D01,
        0x616F00, 0x1F8B00, 0x01980C, 0x00934B,
        0x00819B, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x63B4FF, 0x9B91FF, 0xD377FF,
        0xEF6AFF, 0xF968C0, 0xF97D6C, 0xED9B2D,
        0xBDBD16, 0x7CDA1C, 0x4BE847, 0x35E591,
        0x3FD9DD, 0x606060, 0x000000, 0x000000,
        0xFFFFFF, 0xACE7FF, 0xD5CDFF, 0xEDBAFF,
        0xF8B0FF, 0xFEB0EC, 0xFDBDB5, 0xF9D28E,
        0xE8EB7C, 0xBBF382, 0x99F7A2, 0x8AF5D0,
        0x92F4F1, 0xBEBEBE, 0x000000, 0x000000 } },
    { "composite-direct-fbx", "Composite Direct (FBX)",
      { 0x656565, 0x00127D, 0x18008E, 0x360082,
        0x56005D, 0x5A0018, 0x4F0500, 0x381900,
        0x1D3100, 0x003D00, 0x004100, 0x003B17,
        0x002E55, 0x000000, 0x000000, 0x000000,
        0xAFAFAF, 0x194EC8, 0x472FE3, 0x6B1FD7,
        0x931BAE, 0x9E1A5E, 0x993200, 0x7B4B00,
        0x5B6700, 0x267A00, 0x008200, 0x007A3E,
        0x006E8A, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x64A9FF, 0x8E89FF, 0xB676FF,
        0xE06FFF, 0xEF6CC4, 0xF0806A, 0xD8982C,
        0xB9B40A, 0x83CB0C, 0x5BD63F, 0x4AD17E,
        0x4DC7CB, 0x4C4C4C, 0x000000, 0x000000,
        0xFFFFFF, 0xC7E5FF, 0xD9D9FF, 0xE9D1FF,
        0xF9CEFF, 0xFFCCF1, 0xFFD4CB, 0xF8DFB1,
        0xEDEAA4, 0xD6F4A4, 0xC5F8B8, 0xBEF6D3,
        0xBFF1F1, 0xB9B9B9, 0x000000, 0x000000 } },
    { "nes-classic-fbx", "NES Classic (FBX)",
      { 0x616161, 0x000088, 0x1f0d99, 0x371379,
        0x561260, 0x5d0010, 0x520e00, 0x3a2308,
        0x21350c, 0x0d410e, 0x174417, 0x003a1f,
        0x002f57, 0x000000, 0x000000, 0x000000,
        0xaaaaaa, 0x0d4dc4, 0x4b24de, 0x6912cf,
        0x9014ad, 0x9d1c48, 0x923404, 0x735005,
        0x5d6913, 0x167a11, 0x138008, 0x127649,
        0x1c6691, 0x000000, 0x000000, 0x000000,
        0xfcfcfc, 0x639afc, 0x8a7efc, 0xb06afc,
        0xdd6df2, 0xe771ab, 0xe38658, 0xcc9e22,
        0xa8b100, 0x72c100, 0x5acd4e, 0x34c28e,
        0x4fbece, 0x424242, 0x000000, 0x000000,
        0xfcfcfc, 0xbed4fc, 0xcacafc, 0xd9c4fc,
        0xecc1fc, 0xfac3e7, 0xf7cec3, 0xe2cda7,
        0xdadb9c, 0xc8e39e, 0xbfe5b8, 0xb2ebc8,
        0xb7e5eb, 0xacacac, 0x000000, 0x000000 } },
    { "ntsc-hardware-fbx", "NTSC Hardware (FBX)",
      { 0x6A6D6A, 0x001380, 0x1E008A, 0x39007A,
        0x550056, 0x5A0018, 0x4F1000, 0x382100,
        0x213300, 0x003D00, 0x004000, 0x003924,
        0x002E55, 0x000000, 0x000000, 0x000000,
        0xB9BCB9, 0x1850C7, 0x4B30E3, 0x7322D6,
        0x951FA9, 0x9D285C, 0x963C00, 0x7A5100,
        0x5B6700, 0x227700, 0x027E02, 0x007645,
        0x006E8A, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x68A6FF, 0x9299FF, 0xB085FF,
        0xD975FD, 0xE377B9, 0xE58D68, 0xCFA22C,
        0xB3AF0C, 0x7BC211, 0x55CA47, 0x46CB81,
        0x47C1C5, 0x4A4D4A, 0x000000, 0x000000,
        0xFFFFFF, 0xCCEAFF, 0xDDDEFF, 0xECDAFF,
        0xF8D7FE, 0xFCD6F5, 0xFDDBCF, 0xF9E7B5,
        0xF1F0AA, 0xDAFAA9, 0xC9FFBC, 0xC3FBD7,
        0xC4F6F6, 0xBEC1BE, 0x000000, 0x000000 } },
    { "pvm-style-d93-fbx", "PVM Style D93 (FBX)",
      { 0x696B63, 0x001774, 0x1E0087, 0x340073,
        0x560057, 0x5E0013, 0x531A00, 0x3B2400,
        0x243000, 0x063A00, 0x003F00, 0x003B1E,
        0x00334E, 0x000000, 0x000000, 0x000000,
        0xB9BBB3, 0x1453B9, 0x4D2CDA, 0x671EDE,
        0x98189C, 0x9D2344, 0xA03E00, 0x8D5500,
        0x656D00, 0x2C7900, 0x008100, 0x007D42,
        0x00788A, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x69A8FF, 0x9691FF, 0xB28AFA,
        0xEA7DFA, 0xF37BC7, 0xF28E59, 0xE6AD27,
        0xD7C805, 0x90DF07, 0x64E53C, 0x45E27D,
        0x48D5D9, 0x4E5048, 0x000000, 0x000000,
        0xFFFFFF, 0xD2EAFF, 0xE2E2FF, 0xE9D8FF,
        0xF5D2FF, 0xF8D9EA, 0xFADEB9, 0xF9E89B,
        0xF3F28C, 0xD3FA91, 0xB8FCA8, 0xAEFACA,
        0xCAF3F3, 0xBEC0B8, 0x000000, 0x000000 } },
    { "smooth-fbx", "Smooth (FBX)",
      { 0x6A6D6A, 0x001380, 0x1E008A, 0x39007A,
        0x550056, 0x5A0018, 0x4F1000, 0x3D1C00,
        0x253200, 0x003D00, 0x004000, 0x003924,
        0x002E55, 0x000000, 0x000000, 0x000000,
        0xB9BCB9, 0x1850C7, 0x4B30E3, 0x7322D6,
        0x951FA9, 0x9D285C, 0x983700, 0x7F4C00,
        0x5E6400, 0x227700, 0x027E02, 0x007645,
        0x006E8A, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x68A6FF, 0x8C9CFF, 0xB586FF,
        0xD975FD, 0xE377B9, 0xE58D68, 0xD49D29,
        0xB3AF0C, 0x7BC211, 0x55CA47, 0x46CB81,
        0x47C1C5, 0x4A4D4A, 0x000000, 0x000000,
        0xFFFFFF, 0xCCEAFF, 0xDDDEFF, 0xECDAFF,
        0xF8D7FE, 0xFCD6F5, 0xFDDBCF, 0xF9E7B5,
        0xF1F0AA, 0xDAFAA9, 0xC9FFBC, 0xC3FBD7,
        0xC4F6F6, 0xBEC1BE, 0x000000, 0x000000 } },
    { "unsaturated-final", "Unsaturated Final (FBX)",
      { 0x676767, 0x001F8E, 0x23069E, 0x40008E,
        0x600067, 0x67001C, 0x5B1000, 0x432500,
        0x313400, 0x074800, 0x004F00, 0x004622,
        0x003A61, 0x000000, 0x000000, 0x000000,
        0xB3B3B3, 0x205ADF, 0x5138FB, 0x7A27EE,
        0xA520C2, 0xB0226B, 0xAD3702, 0x8D5600,
        0x6E7000, 0x2E8A00, 0x069200, 0x008A47,
        0x037B9B, 0x101010, 0x000000, 0x000000,
        0xFFFFFF, 0x62AEFF, 0x918BFF, 0xBC78FF,
        0xE96EFF, 0xFC6CCD, 0xFA8267, 0xE29B26,
        0xC0B901, 0x84D200, 0x58DE38, 0x46D97D,
        0x49CED2, 0x494949, 0x000000, 0x000000,
        0xFFFFFF, 0xC1E3FF, 0xD5D4FF, 0xE7CCFF,
        0xFBC9FF, 0xFFC7F0, 0xFFD0C5, 0xF8DAAA,
        0xEBE69A, 0xD1F19A, 0xBEF7AF, 0xB6F4CD,
        0xB7F0EF, 0xB2B2B2, 0x000000, 0x000000 } },
    { "yuv-v3", "YUV-V3 (FBX)",
      { 0x666666, 0x002A88, 0x1412A7, 0x3B00A4,
        0x5C007E, 0x6E0040, 0x6C0700, 0x561D00,
        0x333500, 0x0C4800, 0x005200, 0x004C18,
        0x003E5B, 0x000000, 0x000000, 0x000000,
        0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE,
        0xA01ACC, 0xB71E7B, 0xB53120, 0x994E00,
        0x6B6D00, 0x388700, 0x0D9300, 0x008C47,
        0x007AA0, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x64B0FF, 0x9290FF, 0xC676FF,
        0xF26AFF, 0xFF6ECC, 0xFF8170, 0xEA9E22,
        0xBCBE00, 0x88D800, 0x5CE430, 0x45E082,
        0x48CDDE, 0x4F4F4F, 0x000000, 0x000000,
        0xFFFFFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF,
        0xFAC2FF, 0xFFC4EA, 0xFFCCC5, 0xF7D8A5,
        0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC,
        0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000 } },
    { "nescap", "NESCAP (RGBSource)",
      { 0x646365, 0x001580, 0x1D0090, 0x380082,
        0x56005D, 0x5A001A, 0x4F0900, 0x381B00,
        0x1E3100, 0x003D00, 0x004100, 0x003A1B,
        0x002F55, 0x000000, 0x000000, 0x000000,
        0xAFADAF, 0x164BCA, 0x472AE7, 0x6B1BDB,
        0x9617B0, 0x9F185B, 0x963001, 0x7B4800,
        0x5A6600, 0x237800, 0x017F00, 0x00783D,
        0x006C8C, 0x000000, 0x000000, 0x000000,
        0xFFFFFF, 0x60A6FF, 0x8F84FF, 0xB473FF,
        0xE26CFF, 0xF268C3, 0xEF7E61, 0xD89527,
        0xBAB307, 0x81C807, 0x57D43D, 0x47CF7E,
        0x4BC5CD, 0x4C4B4D, 0x000000, 0x000000,
        0xFFFFFF, 0xC2E0FF, 0xD5D2FF, 0xE3CBFF,
        0xF7C8FF, 0xFEC6EE, 0xFECEC6, 0xF6D7AE,
        0xE9E49F, 0xD3ED9D, 0xC0F2B2, 0xB9F1CC,
        0xBAEDED, 0xBAB9BB, 0x000000, 0x000000 } },
    { "wavebeam", "Wavebeam (Nakedarthur)",
      { 0X6B6B6B, 0X001B88, 0X21009A, 0X40008C,
        0X600067, 0X64001E, 0X590800, 0X481600,
        0X283600, 0X004500, 0X004908, 0X00421D,
        0X003659, 0X000000, 0X000000, 0X000000,
        0XB4B4B4, 0X1555D3, 0X4337EF, 0X7425DF,
        0X9C19B9, 0XAC0F64, 0XAA2C00, 0X8A4B00,
        0X666B00, 0X218300, 0X008A00, 0X008144,
        0X007691, 0X000000, 0X000000, 0X000000,
        0XFFFFFF, 0X63B2FF, 0X7C9CFF, 0XC07DFE,
        0XE977FF, 0XF572CD, 0XF4886B, 0XDDA029,
        0XBDBD0A, 0X89D20E, 0X5CDE3E, 0X4BD886,
        0X4DCFD2, 0X525252, 0X000000, 0X000000,
        0XFFFFFF, 0XBCDFFF, 0XD2D2FF, 0XE1C8FF,
        0XEFC7FF, 0XFFC3E1, 0XFFCAC6, 0XF2DAAD,
        0XEBE3A0, 0XD2EDA2, 0XBCF4B4, 0XB5F1CE,
        0XB6ECF1, 0XBFBFBF, 0X000000, 0X000000 } },
    { "loopy", "Loopy's palette",
      { 0x757575, 0x271b8f, 0x0000ab, 0x47009f,
        0x8f0077, 0xab0013, 0xa70000, 0x7f0b00,
        0x432f00, 0x004700, 0x005100, 0x003f17,
        0x1b3f5f, 0x000000, 0x000000, 0x000000,
        0xbcbcbc, 0x0073ef, 0x233bef, 0x8300f3,
        0xbf00bf, 0xe7005b, 0xdb2b00, 0xcb4f0f,
        0x8b7300, 0x009700, 0x00ab00, 0x00933b,
        0x00838b, 0x000000, 0x000000, 0x000000,
        0xffffff, 0x3fbfff, 0x5f97ff, 0xa78bfd,
        0xf77bff, 0xff77b7, 0xff7763, 0xff9b3b,
        0xf3bf3f, 0x83d313, 0x4fdf4b, 0x58f898,
        0x00ebdb, 0x000000, 0x000000, 0x000000,
        0xffffff, 0xabe7ff, 0xc7d7ff, 0xd7cbff,
        0xffc7ff, 0xffc7db, 0xffbfb3, 0xffdbab,
        0xffe7a3, 0xe3ffa3, 0xabf3bf, 0xb3ffcf,
        0x9ffff3, 0x000000, 0x000000, 0x000000 } },
    { "quor", "Quor's palette",
      { 0x3f3f3f, 0x001f3f, 0x00003f, 0x1f003f,
        0x3f003f, 0x3f0020, 0x3f0000, 0x3f2000,
        0x3f3f00, 0x203f00, 0x003f00, 0x003f20,
        0x003f3f, 0x000000, 0x000000, 0x000000,
        0x7f7f7f, 0x405f7f, 0x40407f, 0x5f407f,
        0x7f407f, 0x7f4060, 0x7f4040, 0x7f6040,
        0x7f7f40, 0x607f40, 0x407f40, 0x407f60,
        0x407f7f, 0x000000, 0x000000, 0x000000,
        0xbfbfbf, 0x809fbf, 0x8080bf, 0x9f80bf,
        0xbf80bf, 0xbf80a0, 0xbf8080, 0xbfa080,
        0xbfbf80, 0xa0bf80, 0x80bf80, 0x80bfa0,
        0x80bfbf, 0x000000, 0x000000, 0x000000,
        0xffffff, 0xc0dfff, 0xc0c0ff, 0xdfc0ff,
        0xffc0ff, 0xffc0e0, 0xffc0c0, 0xffe0c0,
        0xffffc0, 0xe0ffc0, 0xc0ffc0, 0xc0ffe0,
        0xc0ffff, 0x000000, 0x000000, 0x000000 } },
    { "chris", "Chris Covell's palette",
      { 0x808080, 0x003DA6, 0x0012B0, 0x440096,
        0xA1005E, 0xC70028, 0xBA0600, 0x8C1700,
        0x5C2F00, 0x104500, 0x054A00, 0x00472E,
        0x004166, 0x000000, 0x050505, 0x050505,
        0xC7C7C7, 0x0077FF, 0x2155FF, 0x8237FA,
        0xEB2FB5, 0xFF2950, 0xFF2200, 0xD63200,
        0xC46200, 0x358000, 0x058F00, 0x008A55,
        0x0099CC, 0x212121, 0x090909, 0x090909,
        0xFFFFFF, 0x0FD7FF, 0x69A2FF, 0xD480FF,
        0xFF45F3, 0xFF618B, 0xFF8833, 0xFF9C12,
        0xFABC20, 0x9FE30E, 0x2BF035, 0x0CF0A4,
        0x05FBFF, 0x5E5E5E, 0x0D0D0D, 0x0D0D0D,
        0xFFFFFF, 0xA6FCFF, 0xB3ECFF, 0xDAABEB,
        0xFFA8F9, 0xFFABB3, 0xFFD2B0, 0xFFEFA6,
        0xFFF79C, 0xD7E895, 0xA6EDAF, 0xA2F2DA,
        0x99FFFC, 0xDDDDDD, 0x111111, 0x111111 } },
    { "matt", "Matthew Conte's palette",
      { 0x808080, 0x0000bb, 0x3700bf, 0x8400a6,
        0xbb006a, 0xb7001e, 0xb30000, 0x912600,
        0x7b2b00, 0x003e00, 0x00480d, 0x003c22,
        0x002f66, 0x000000, 0x050505, 0x050505,
        0xc8c8c8, 0x0059ff, 0x443cff, 0xb733cc,
        0xff33aa, 0xff375e, 0xff371a, 0xd54b00,
        0xc46200, 0x3c7b00, 0x1e8415, 0x009566,
        0x0084c4, 0x111111, 0x090909, 0x090909,
        0xffffff, 0x0095ff, 0x6f84ff, 0xd56fff,
        0xff77cc, 0xff6f99, 0xff7b59, 0xff915f,
        0xffa233, 0xa6bf00, 0x51d96a, 0x4dd5ae,
        0x00d9ff, 0x666666, 0x0d0d0d, 0x0d0d0d,
        0xffffff, 0x84bfff, 0xbbbbff, 0xd0bbff,
        0xffbfea, 0xffbfcc, 0xffc4b7, 0xffccae,
        0xffd9a2, 0xcce199, 0xaeeeb7, 0xaaf7ee,
        0xb3eeff, 0xdddddd, 0x111111, 0x111111 } },
    { "pasofami", "PasoFami/99 palette",
      { 0x7f7f7f, 0x0000ff, 0x0000bf, 0x472bbf,
        0x970087, 0xab0023, 0xab1300, 0x8b1700,
        0x533000, 0x007800, 0x006b00, 0x005b00,
        0x004358, 0x000000, 0x000000, 0x000000,
        0xbfbfbf, 0x0078f8, 0x0058f8, 0x6b47ff,
        0xdb00cd, 0xe7005b, 0xf83800, 0xe75f13,
        0xaf7f00, 0x00b800, 0x00ab00, 0x00ab47,
        0x008b8b, 0x000000, 0x000000, 0x000000,
        0xf8f8f8, 0x3fbfff, 0x6b88ff, 0x9878f8,
        0xf878f8, 0xf85898, 0xf87858, 0xffa347,
        0xf8b800, 0xb8f818, 0x5bdb57, 0x58f898,
        0x00ebdb, 0x787878, 0x000000, 0x000000,
        0xffffff, 0xa7e7ff, 0xb8b8f8, 0xd8b8f8,
        0xf8b8f8, 0xfba7c3, 0xf0d0b0, 0xffe3ab,
        0xfbdb7b, 0xd8f878, 0xb8f8b8, 0xb8f8d8,
        0x00ffff, 0xf8d8f8, 0x000000, 0x000000 } },
    { "crashman", "CrashMan's palette",
      { 0x585858, 0x001173, 0x000062, 0x472bbf,
        0x970087, 0x910009, 0x6f1100, 0x4c1008,
        0x371e00, 0x002f00, 0x005500, 0x004d15,
        0x002840, 0x000000, 0x000000, 0x000000,
        0xa0a0a0, 0x004499, 0x2c2cc8, 0x590daa,
        0xae006a, 0xb00040, 0xb83418, 0x983010,
        0x704000, 0x308000, 0x207808, 0x007b33,
        0x1c6888, 0x000000, 0x000000, 0x000000,
        0xf8f8f8, 0x267be1, 0x5870f0, 0x9878f8,
        0xff73c8, 0xf060a8, 0xd07b37, 0xe09040,
        0xf8b300, 0x8cbc00, 0x40a858, 0x58f898,
        0x00b7bf, 0x787878, 0x000000, 0x000000,
        0xffffff, 0xa7e7ff, 0xb8b8f8, 0xd8b8f8,
        0xe6a6ff, 0xf29dc4, 0xf0c0b0, 0xfce4b0,
        0xe0e01e, 0xd8f878, 0xc0e890, 0x95f7c8,
        0x98e0e8, 0xf8d8f8, 0x000000, 0x000000 } },
    { "mess", "MESS palette",
      { 0x747474, 0x24188c, 0x0000a8, 0x44009c,
        0x8c0074, 0xa80010, 0xa40000, 0x7c0800,
        0x402c00, 0x004400, 0x005000, 0x003c14,
        0x183c5c, 0x000000, 0x000000, 0x000000,
        0xbcbcbc, 0x0070ec, 0x2038ec, 0x8000f0,
        0xbc00bc, 0xe40058, 0xd82800, 0xc84c0c,
        0x887000, 0x009400, 0x00a800, 0x009038,
        0x008088, 0x000000, 0x000000, 0x000000,
        0xfcfcfc, 0x3cbcfc, 0x5c94fc, 0x4088fc,
        0xf478fc, 0xfc74b4, 0xfc7460, 0xfc9838,
        0xf0bc3c, 0x80d010, 0x4cdc48, 0x58f898,
        0x00e8d8, 0x000000, 0x000000, 0x000000,
        0xfcfcfc, 0xa8e4fc, 0xc4d4fc, 0xd4c8fc,
        0xfcc4fc, 0xfcc4d8, 0xfcbcb0, 0xfcd8a8,
        0xfce4a0, 0xe0fca0, 0xa8f0bc, 0xb0fccc,
        0x9cfcf0, 0x000000, 0x000000, 0x000000 } },
    { "pal-kinopio", "PAL (Kinopio)",
      { 0x666666, 0x00267b, 0x0812ac, 0x2e08b0,
        0x5f007f, 0x710047, 0x730600, 0x5c1200,
        0x312400, 0x033100, 0x003b00, 0x003a00,
        0x00304b, 0x000000, 0x000000, 0x000000,
        0xafb0af, 0x0058c3, 0x333eff, 0x6232ff,
        0x9d25c7, 0xb52581, 0xb73015, 0x9b3f00,
        0x675500, 0x2c6500, 0x007100, 0x00700f,
        0x006486, 0x000000, 0x000000, 0x000000,
        0xffffff, 0x4ea9ff, 0x8792ff, 0xb187ff,
        0xe97bff, 0xff7bce, 0xff856b, 0xe69227,
        0xb6a600, 0x82b400, 0x50c027, 0x3abf67,
        0x3ab4d2, 0x50504f, 0x000000, 0x000000,
        0xffffff, 0xbde2ff, 0xd4d9ff, 0xe5d4ff,
        0xfcd0ff, 0xffcff1, 0xffd3c7, 0xfbd8ab,
        0xe5e196, 0xd1e798, 0xbdecab, 0xb5ebc5,
        0xb5e6f2, 0xbbbbba, 0x000000, 0x000000 } },
    { "zaphod-cv", "Zaphod's VS Castlevania palette",
      { 0x7f7f7f, 0xffa347, 0x0000bf, 0x472bbf,
        0x970087, 0xf85898, 0xab1300, 0xf8b8f8,
        0xbf0000, 0x007800, 0x006b00, 0x005b00,
        0xffffff, 0x9878f8, 0x000000, 0x000000,
        0xbfbfbf, 0x0078f8, 0xab1300, 0x6b47ff,
        0x00ae00, 0xe7005b, 0xf83800, 0x7777ff,
        0xaf7f00, 0x00b800, 0x00ab00, 0x00ab47,
        0x008b8b, 0x000000, 0x000000, 0x472bbf,
        0xf8f8f8, 0xffe3ab, 0xf87858, 0x9878f8,
        0x0078f8, 0xf85898, 0xbfbfbf, 0xffa347,
        0xc800c8, 0xb8f818, 0x7f7f7f, 0x007800,
        0x00ebdb, 0x000000, 0x000000, 0xffffff,
        0xffffff, 0xa7e7ff, 0x5bdb57, 0xe75f13,
        0x004358, 0x0000ff, 0xe7005b, 0x00b800,
        0xfbdb7b, 0xd8f878, 0x8b1700, 0xffe3ab,
        0x00ffff, 0xab0023, 0x000000, 0x000000 } },
    { "zaphod-smb", "Zaphod's VS SMB palette",
      { 0x626a00, 0x0000ff, 0x006a77, 0x472bbf,
        0x970087, 0xab0023, 0xab1300, 0xb74800,
        0xa2a2a2, 0x007800, 0x006b00, 0x005b00,
        0xffd599, 0xffff00, 0x009900, 0x000000,
        0xff66ff, 0x0078f8, 0x0058f8, 0x6b47ff,
        0x000000, 0xe7005b, 0xf83800, 0xe75f13,
        0xaf7f00, 0x00b800, 0x5173ff, 0x00ab47,
        0x008b8b, 0x000000, 0x91ff88, 0x000088,
        0xf8f8f8, 0x3fbfff, 0x6b0000, 0x4855f8,
        0xf878f8, 0xf85898, 0x595958, 0xff009d,
        0x002f2f, 0xb8f818, 0x5bdb57, 0x58f898,
        0x00ebdb, 0x787878, 0x000000, 0x000000,
        0xffffff, 0xa7e7ff, 0x590400, 0xbb0000,
        0xf8b8f8, 0xfba7c3, 0xffffff, 0x00e3e1,
        0xfbdb7b, 0xffae00, 0xb8f8b8, 0xb8f8d8,
        0x00ff00, 0xf8d8f8, 0xffaaaa, 0x004000 } },
    { "vs-drmar", "VS Dr. Mario palette",
      { 0x5f97ff, 0x000000, 0x000000, 0x47009f,
        0x00ab00, 0xffffff, 0xabe7ff, 0x000000,
        0x000000, 0x000000, 0x000000, 0x000000,
        0xe7005b, 0x000000, 0x000000, 0x000000,
        0x5f97ff, 0x000000, 0x000000, 0x000000,
        0x000000, 0x8b7300, 0xcb4f0f, 0x000000,
        0xbcbcbc, 0x000000, 0x000000, 0x000000,
        0x000000, 0x000000, 0x000000, 0x000000,
        0x00ebdb, 0x000000, 0x000000, 0x000000,
        0x000000, 0xff9b3b, 0x000000, 0x000000,
        0x83d313, 0x000000, 0x3fbfff, 0x000000,
        0x0073ef, 0x000000, 0x000000, 0x000000,
        0x00ebdb, 0x000000, 0x000000, 0x000000,
        0x000000, 0x000000, 0xf3bf3f, 0x000000,
        0x005100, 0x000000, 0xc7d7ff, 0xffdbab,
        0x000000, 0x000000, 0x000000, 0x000000 } },
    { "vs-cv", "VS Castlevania palette",
      { 0xaf7f00, 0xffa347, 0x008b8b, 0x472bbf,
        0x970087, 0xf85898, 0xab1300, 0xf8b8f8,
        0xf83800, 0x007800, 0x006b00, 0x005b00,
        0xffffff, 0x9878f8, 0x00ab00, 0x000000,
        0xbfbfbf, 0x0078f8, 0xab1300, 0x6b47ff,
        0x000000, 0xe7005b, 0xf83800, 0x6b88ff,
        0xaf7f00, 0x00b800, 0x6b88ff, 0x00ab47,
        0x008b8b, 0x000000, 0x000000, 0x472bbf,
        0xf8f8f8, 0xffe3ab, 0xf87858, 0x9878f8,
        0x0078f8, 0xf85898, 0xbfbfbf, 0xffa347,
        0x004358, 0xb8f818, 0x7f7f7f, 0x007800,
        0x00ebdb, 0x000000, 0x000000, 0xffffff,
        0xffffff, 0xa7e7ff, 0x5bdb57, 0x6b88ff,
        0x004358, 0x0000ff, 0xe7005b, 0x00b800,
        0xfbdb7b, 0xffa347, 0x8b1700, 0xffe3ab,
        0xb8f818, 0xab0023, 0x000000, 0x007800 } },
    { "vs-smb", "VS SMB/VS Ice Climber palette",
      { 0xaf7f00, 0x0000ff, 0x008b8b, 0x472bbf,
        0x970087, 0xab0023, 0x0000ff, 0xe75f13,
        0xbfbfbf, 0x007800, 0x5bdb57, 0x005b00,
        0xf0d0b0, 0xffe3ab, 0x00ab00, 0x000000,
        0xbfbfbf, 0x0078f8, 0x0058f8, 0x6b47ff,
        0x000000, 0xe7005b, 0xf83800, 0xf87858,
        0xaf7f00, 0x00b800, 0x6b88ff, 0x00ab47,
        0x008b8b, 0x000000, 0x000000, 0x3fbfff,
        0xf8f8f8, 0x006b00, 0x8b1700, 0x9878f8,
        0x6b47ff, 0xf85898, 0x7f7f7f, 0xe7005b,
        0x004358, 0xb8f818, 0x0078f8, 0x58f898,
        0x00ebdb, 0xfbdb7b, 0x000000, 0x000000,
        0xffffff, 0xa7e7ff, 0xb8b8f8, 0xf83800,
        0xf8b8f8, 0xfba7c3, 0xffffff, 0x00ffff,
        0xfbdb7b, 0xffa347, 0xb8f8b8, 0xb8f8d8,
        0xb8f818, 0xf8d8f8, 0x000000, 0x007800 } }
};

/************************************/
/* gsKit Variables                  */
/************************************/
GSTEXTURE NES_TEX;
GSTEXTURE BG_TEX;
GSTEXTURE MENU_TEX;

u8 menutex = 0;
u8 bgtex = 0;

extern GSGLOBAL *gsGlobal;
//unsigned int ps2palette[256];
/* Normal loopy palette
u32 NesPalette[ 64 ] =
{
    0x757575, 0x271b8f, 0x0000ab, 0x47009f, 0x8f0077, 0xab0013, 0xa70000, 0x7f0b00,
    0x432f00, 0x004700, 0x005100, 0x003f17, 0x1b3f5f, 0x000000, 0x000000, 0x000000,
    0xbcbcbc, 0x0073ef, 0x233bef, 0x8300f3, 0xbf00bf, 0xe7005b, 0xdb2b00, 0xcb4f0f,
    0x8b7300, 0x009700, 0x00ab00, 0x00933b, 0x00838b, 0x000000, 0x000000, 0x000000,
    0xffffff, 0x3fbfff, 0x5f97ff, 0xa78bfd, 0xf77bff, 0xff77b7, 0xff7763, 0xff9b3b,
    0xf3bf3f, 0x83d313, 0x4fdf4b, 0x58f898, 0x00ebdb, 0x000000, 0x000000, 0x000000,
    0xffffff, 0xabe7ff, 0xc7d7ff, 0xd7cbff, 0xffc7ff, 0xffc7db, 0xffbfb3, 0xffdbab,
    0xffe7a3, 0xe3ffa3, 0xabf3bf, 0xb3ffcf, 0x9ffff3, 0x000000, 0x000000, 0x000000
};
u32 NesPalette[ 64 ] = // Modified palette for GS
{
    0x757575, 0x271b8f, 0x0000ab, 0x47009f, 0x8f0077, 0xab0013, 0xa70000, 0x7f0b00, // 1 start
    0xbcbcbc, 0x0073ef, 0x233bef, 0x8300f3, 0xbf00bf, 0xe7005b, 0xdb2b00, 0xcb4f0f, // 2 start
    0x432f00, 0x004700, 0x005100, 0x003f17, 0x1b3f5f, 0x000000, 0x000000, 0x000000, // 1 finish
    0x8b7300, 0x009700, 0x00ab00, 0x00933b, 0x00838b, 0x000000, 0x000000, 0x000000, // 2 finish
    0xffffff, 0x3fbfff, 0x5f97ff, 0xa78bfd, 0xf77bff, 0xff77b7, 0xff7763, 0xff9b3b, // 3 start
    0xffffff, 0xabe7ff, 0xc7d7ff, 0xd7cbff, 0xffc7ff, 0xffc7db, 0xffbfb3, 0xffdbab, // 4 start
    0xf3bf3f, 0x83d313, 0x4fdf4b, 0x58f898, 0x00ebdb, 0x000000, 0x000000, 0x000000, // 3 finish
    0xffe7a3, 0xe3ffa3, 0xabf3bf, 0xb3ffcf, 0x9ffff3, 0x000000, 0x000000, 0x000000  // 4 finish
};
u32 NesPalette[64] =  // "AspiringSquire's NES Palette
{
    0x6c6c6c, 0x00268e, 0x0000a8, 0x400090, 0x700070, 0x780040, 0x700000, 0x621600,
    0xbababa, 0x205cdc, 0x3838ff, 0x8020f0, 0xc000c0, 0xd01474, 0xd02020, 0xac4014,
    0x442400, 0x343400, 0x005000, 0x004444, 0x004060, 0x000000, 0x101010, 0x101010,
    0x7c5400, 0x586400, 0x008800, 0x007468, 0x00749c, 0x202020, 0x101010, 0x101010,
    0xffffff, 0x4ca0ff, 0x8888ff, 0xc06cff, 0xff50ff, 0xff64b8, 0xff7878, 0xff9638,
    0xffffff, 0xb0d4ff, 0xc4c4ff, 0xe8b8ff, 0xffb0ff, 0xffb8e8, 0xffc4c4, 0xffd4a8,
    0xdbab00, 0xa2ca20, 0x4adc4a, 0x2ccca4, 0x1cc2ea, 0x585858, 0x101010, 0x101010,
    0xffe890, 0xf0f4a4, 0xc0ffc0, 0xacf4f0, 0xa0e8ff, 0xc2c2c2, 0x202020, 0x101010
};*/

/************************************/
/* Prototypes                       */
/************************************/
extern void Set_NESInput();
extern  int Get_NESInput();

static  int PS2_LoadGame(char *path);
static void SetupNESTexture();
       void SetupNESClut();
       void SetupNESGS();
static void DoFun();
/***********************************/
/* Sound                           */
/***********************************/
#define SAMPLERATES_SIZE 4
static  int SND_sampleRates[SAMPLERATES_SIZE] = { 0, 11025, 22050, 44100 };
static void SND_Init();
       void SND_SetNextSampleRate();
        int SND_GetCurrSampleRate();

int main(int argc, char *argv[])
{
    int ret, sometime;
    char *temp;
    char boot_path[256];
    char *p;

    mpartitions[0][0] = 0;
    mpartitions[1][0] = 0;
    mpartitions[2][0] = 0;

    // Setup PS2 here
    InitPS2();
    setupPS2Pad();

    // Init Settings
    strcpy(boot_path, argv[0]);
    if (((p = strrchr(boot_path, '/')) == NULL) && ((p = strrchr(boot_path, '\\')) == NULL))
        p = strrchr(boot_path, ':');
    if (p != NULL)
        *(p+1) = 0;
    // The above cuts away the ELF filename from argv[0], leaving a pure path
    if (!strncmp(boot_path, "hdd0:", 5)) {
        char hdd_path[256];
        char *t;
        sprintf(hdd_path, "%s", boot_path+5);
        t = strchr(hdd_path, ':');
        if (t != NULL)
            *t = 0;
        //hdd0:HDDPATH:pfs:PFSPATH
        sprintf(boot_path, "hdd0:/%s%s", hdd_path, boot_path+5+strlen(hdd_path)+5); //
        if (boot_path[5+1+strlen(hdd_path)] != '/')
            sprintf(boot_path, "hdd0:/%s/", hdd_path);
        //hdd0:/HDDPATHPFSPATH
    }

    Default_Global_CNF();
    Load_Global_CNF(boot_path);

    for (ret = 0; ret < 3; ret++) {
        sometime = 0x10000;
        while (sometime--) asm("nop\nnop\nnop\nnop");
    }

    SetupGSKit();

    gsKit_init_screen(gsGlobal); // Initialize everything
    //init_custom_screen(); // Init user screen settings

    loadFont(0);

    // Init Skin
    FCEUSkin.textcolor = 0;
    Load_Skin_CNF(Settings.skinpath);
    for (ret = 0; ret < 3; ret++) {
        sometime = 0x10000;
        while (sometime--) asm("nop\nnop\nnop\nnop");
    }
    if (!FCEUSkin.textcolor) { // Initialize default values
        printf("Load Skin Failed\n");
        Default_Skin_CNF();
    }

    // Setup GUI Textures
    jpgData *Jpg;
    u8 *ImgData;
    if (strstr(FCEUSkin.bgTexture, ".png") != NULL) {
         if (gsKit_texture_png(gsGlobal, &BG_TEX, FCEUSkin.bgTexture) < 0) {
            printf("Error with browser background png!\n");
            bgtex = 1;
         }
    }
    else if (strstr(FCEUSkin.bgTexture, ".jpg") || strstr(FCEUSkin.bgTexture, ".jpeg") != NULL){
        //if (gsKit_texture_jpeg(gsGlobal, &BG_TEX, FCEUSkin.bgTexture) < 0) {
        FILE *File = fopen(FCEUSkin.bgTexture, "r");
        if (File != NULL) {
            Jpg = jpgOpenFILE(File, JPG_WIDTH_FIX); // > 0)
            ImgData = malloc(Jpg->width * Jpg->height * (Jpg->bpp / 8)); // > 0)
            jpgReadImage(Jpg, ImgData);
            BG_TEX.PSM = GS_PSM_CT24;
            BG_TEX.Clut = NULL;
            BG_TEX.VramClut = 0;
            BG_TEX.Width = Jpg->width;
            BG_TEX.Height = Jpg->height;
            BG_TEX.Filter = GS_FILTER_LINEAR;
            BG_TEX.Mem = memalign(128, gsKit_texture_size_ee(BG_TEX.Width, BG_TEX.Height, BG_TEX.PSM));
            BG_TEX.Mem = (void*)ImgData;
            BG_TEX.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(BG_TEX.Width, BG_TEX.Height, BG_TEX.PSM), GSKIT_ALLOC_USERBUFFER);
            gsKit_texture_upload(gsGlobal, &BG_TEX);
            free(BG_TEX.Mem);
        }
        else {
            printf("Error with browser background jpg!\n");
            bgtex = 1;
        }
    }
    else {
        bgtex = 1;
    }

    Jpg = 0;
    ImgData = 0;

    if (strstr(FCEUSkin.bgMenu, ".png") != NULL) {
        if (gsKit_texture_png(gsGlobal, &MENU_TEX, FCEUSkin.bgMenu) == -1) {
            printf("Error with menu background png!\n");
            menutex = 1;
        }
    }
    else if (strstr(FCEUSkin.bgMenu, ".jpg") || strstr(FCEUSkin.bgMenu, ".jpeg") != NULL) {
        //if (gsKit_texture_jpeg(gsGlobal, &MENU_TEX, FCEUSkin.bgMenu) < 0) { // Apparently didn't like the "myps2" libjpg
        FILE *File = fopen(FCEUSkin.bgMenu, "r");
        if (File != NULL) {
            Jpg = jpgOpenFILE(File, JPG_WIDTH_FIX); // > 0)
            ImgData = malloc(Jpg->width * Jpg->height * (Jpg->bpp / 8)); // > 0)
            jpgReadImage(Jpg, ImgData);
            MENU_TEX.PSM = GS_PSM_CT24;
            MENU_TEX.Clut = NULL;
            MENU_TEX.VramClut = 0;
            MENU_TEX.Width = Jpg->width;
            MENU_TEX.Height = Jpg->height;
            MENU_TEX.Filter = GS_FILTER_LINEAR;
            MENU_TEX.Mem = memalign(128, gsKit_texture_size_ee(MENU_TEX.Width, MENU_TEX.Height, MENU_TEX.PSM));
            MENU_TEX.Mem = (void*)ImgData;
            MENU_TEX.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(MENU_TEX.Width, MENU_TEX.Height, MENU_TEX.PSM), GSKIT_ALLOC_USERBUFFER);
            gsKit_texture_upload(gsGlobal, &MENU_TEX);
            free(MENU_TEX.Mem);
        }
        else {
            printf("Error with menu background jpg!\n");
            menutex =1;
        }
    }
    else {
        menutex = 1;
    }


    if (!(ret = FCEUI_Initialize())) { // Allocates all memory for FCEU* functions
        printf("FCEUltra did not initialize.\n");
        return 0;
    }

    // Setup FCEUltra here
    FCEUI_SetVidSystem(Settings.emulation); // 0 = ntsc, 1 = pal
    FCEUI_SetGameGenie(1);
    FCEUI_DisableSpriteLimitation(1);
//    FCEUI_SetRenderedLines(8, 231, 0, 239);
//    FCEUI_GetCurrentVidSystem(&srendline, &erendline);
//    totallines = erendline - srendline + 1;

    SND_Init();

    SetupNESTexture();

    // Main emulation loop
    for (;;) {
        strcpy((char *)path, Browser(1, 0));

        if (PS2_LoadGame((char *)path) == 0) {
            continue;
        }

        Set_NESInput();
        SetupNESClut();
        SetupNESGS();

        while (CurGame) // FCEUI_CloseGame turns this false
            DoFun();

#ifdef SOUND_ON
        audsrv_stop_audio();
#endif
        temp = strrchr((char *)path, '/');
        temp++;
        *temp = 0;
    }

    return 0;
}

static int PS2_LoadGame(char *path)
{
    FCEUGI *tmp;

//    CloseGame();
    if ((tmp = FCEUI_LoadGame(path))) {
        printf("Loaded!\n");
        CurGame = tmp;
        return 1;
    }
    else {
        printf("Didn't load!\n");
        return 0;
    }
}

void SetupNESTexture()
{
    // Comments after settings are for regular clut lookup
    // Setup NES_TEX Texture
    NES_TEX.PSM = GS_PSM_T8; //GS_PSM_CT32
    NES_TEX.ClutPSM = GS_PSM_CT32; // Comment out
    NES_TEX.Clut = memalign(128, gsKit_texture_size_ee(16, 16, NES_TEX.ClutPSM)); //NULL
    NES_TEX.VramClut = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(16, 16, NES_TEX.ClutPSM), GSKIT_ALLOC_USERBUFFER); //0
    NES_TEX.Width = 256; // Full size 256x240
    NES_TEX.Height = 240;
    NES_TEX.TBW = 4;
    //NES_TEX.Mem = memalign(128, gsKit_texture_size_ee(NES_TEX.Width, NES_TEX.Height, NES_TEX.PSM));
    NES_TEX.Vram = gsKit_vram_alloc(gsGlobal, gsKit_texture_size(NES_TEX.Width, NES_TEX.Height, NES_TEX.PSM), GSKIT_ALLOC_USERBUFFER);
}

void SetupNESClut()
{
    int i;
    uint8 r, g, b;
    // Default palette
    for (i = 0; i < 256; i++) {
        FCEUD_GetPalette(i, &r, &g, &b);
        NES_TEX.Clut[i] = ((b<<16)|(g<<8)|(r<<0));
    }
    
    if (Settings.current_palette != 0 && CurGame->type != GIT_VSUNI) {
        for (i = 0; i < 64; i++) {
            // 32-bit bgr -< rgb
            r = (palettes[Settings.current_palette - 1].data[i] & 0xff0000) >> 16;
            g = (palettes[Settings.current_palette - 1].data[i] &   0xff00) >>  8;
            b = (palettes[Settings.current_palette - 1].data[i] &     0xff) >>  0;
            //NES_TEX.Clut[i +   0] = ((b<<16)|(g<<8)|(r<<0)); //NES_TEX.Clut = ps2palette;
            //NES_TEX.Clut[i +  64] = ((b<<16)|(g<<8)|(r<<0));
            NES_TEX.Clut[i + 128] = ((b<<16)|(g<<8)|(r<<0));
            //NES_TEX.Clut[i + 192] = ((b<<16)|(g<<8)|(r<<0));
        }
    }

    // Modification for GS
    for (i = 0; i < 256; i++) {
        int modi = i & 63;
        if ((modi >= 8 && modi < 16) || (modi >= 40 && modi < 48)) {
            u32 tmp             = NES_TEX.Clut[i    ];
            NES_TEX.Clut[i    ] = NES_TEX.Clut[i + 8];
            NES_TEX.Clut[i + 8] = tmp;
        }
    }
}

void SetupNESGS()
{
    uint8 r, g, b;
    int v1, v2, y1, y2;
    gsGlobal->DrawOrder = GS_OS_PER;
    gsKit_mode_switch(gsGlobal, GS_PERSISTENT);
    gsKit_queue_reset(gsGlobal->Per_Queue);

    if (Settings.filter) {
        NES_TEX.Filter = GS_FILTER_LINEAR;
    }
    else {
        NES_TEX.Filter = GS_FILTER_NEAREST;
    }

//    gsKit_clear(gsGlobal, GS_SETREG_RGBA(0x00, 0x00, 0x00, 0x00));
    FCEUD_GetPalette(0, &r, &g, &b);
//    r =  (NesPalette[ 0 ] & 0xff0000)>>16;
//    g =  (NesPalette[ 0 ] & 0xff00  )>> 8;
//    b =  (NesPalette[ 0 ] & 0xff    )<< 0;

    gsKit_clear(gsGlobal, GS_SETREG_RGBA(r, g, b, 0x00));
    y1 = 0;
    v1 = 0;
    y2 = NES_TEX.Height*2;
    v2 = NES_TEX.Height;

    int offsetX = 0, width = gsGlobal->Width;

    if (gsGlobal->Mode == GS_MODE_NTSC) {
        v1 = 8;
        v2 = (NES_TEX.Height - 16) + 8;
        y2 = (NES_TEX.Height - 16);

        if (Settings.aspect_ratio == 1) {
            int newWidth = (width * 3 / 4) * 256 / 224;  // 548 if width = 640
            offsetX = (width - newWidth) / 2; // (640 - 548) / 2
            width = newWidth;
        }
    }
    else if (gsGlobal->Mode == GS_MODE_PAL) {
        v1 = 0;
        v2 = NES_TEX.Height;
        y2 = NES_TEX.Height;

        if (Settings.aspect_ratio == 1) {
            offsetX = 80;
            width = 480;
        }
    }
    else if (gsGlobal->Mode == GS_MODE_DTV_480P) {
        v1 = 0;
        v2 = NES_TEX.Height;
        y2 = gsGlobal->Height;

        if (Settings.aspect_ratio == 1) {
            offsetX = 64;
            width = 512;
        }
    }
    if (gsGlobal->Interlace == GS_INTERLACED && (gsGlobal->Mode == GS_MODE_NTSC || gsGlobal->Mode == GS_MODE_PAL))
        y2 = y2*2;
    //gsKit_prim_sprite_striped_texture(gsGlobal, &NES_TEX,  // Thought this might be needed for different modes, but it just looks bad

    gsKit_prim_sprite_texture(gsGlobal, &NES_TEX,
        0 + offsetX,                           /* X1 */
        y1,                                    /* Y1 */
        0,                                     /* U1 */
        v1,                                    /* V1 */
        width + offsetX,                       /* X2 */ // Stretch to screen width
        y2,                                    /* Y2 */ // Stretch to screen height
        NES_TEX.Width,                         /* U2 */
        v2,                                    /* V2 */
        2,                                     /* Z  */
        GS_SETREG_RGBA(0x80, 0x80, 0x80, 0x00) /* RGBA */
    );
}

void RenderFrame(const uint8 *frame)
{
    //int w, h, c;
    //int i;

/*
    for (h = 0; h < 240; h++) { // Correctly displays 256x240 nes screen
        for (w = 0; w < 256; w++) {
            c = (h << 8) + w; // Color index, increments height by 256, then adds width
            NES_TEX.Mem[c] = ps2palette[frame[c]];
        }
    }
*/
    NES_TEX.Mem = (u32 *)frame; // Set frame as NES_TEX.Mem location

    gsKit_texture_upload(gsGlobal, &NES_TEX);

    // Don't swap these lines
    /* vsync and flip buffer */
    gsKit_sync_flip(gsGlobal);

    /* execute render queue */
    gsKit_queue_exec(gsGlobal);
}

#ifdef SOUND_ON
void inline OutputSound(const int32 *tmpsnd, int32 ssize)
{
    // Used as an example from the windows driver
    /*static int16 MBuffer[2 * 96000 / 50];  // * 2 for safety.
    int P;

    if (!bittage) {
        for (P = 0; P < Count; P++)
            *(((uint8*)MBuffer)+P) = ((int8)(Buffer[P]>>8))^128;
        RawWrite(MBuffer, Count);
    }
    else {
        for (P = 0; P < Count; P++)
        MBuffer[P] = Buffer[P];
        //FCEU_printf("Pre: %d\n", RawCanWrite() / 2);
        RawWrite(MBuffer, Count * 2);
        //FCEU_printf("Post: %d\n", RawCanWrite() / 2);
     }*/

    int i;
    s16 ssound[ssize]; // No need for an 2*ssized 8bit array with this

    //audsrv_wait_audio(ssize<<1); // Commented out because the sound buffer is filled at need
    for (i = 0; i < ssize; i++) {
        //something[i] = ((tmpsnd[i]>>8))^128; // For 8bit sound
        ssound[i] = tmpsnd[i];
    }

    audsrv_play_audio((char *)ssound, ssize << 1); //
}
#endif

void FCEUD_Update(const uint8 *XBuf, const int32 *tmpsnd, int32 ssize)
{
    RenderFrame(XBuf);
#ifdef SOUND_ON
    OutputSound(tmpsnd, ssize);
#endif
    if (Get_NESInput()) {
        FCEUI_CloseGame();
        CurGame = 0;
    }
}

static void DoFun()
{
    uint8 *gfx;
    int32 *sound;
    int32 ssize;

    FCEUI_Emulate(&gfx, &sound, &ssize, 0);
    FCEUD_Update(gfx, sound, ssize);
}

static void SND_Init()
{
#ifdef SOUND_ON
    FCEUI_SetSoundVolume(1024);
#else
    FCEUI_SetSoundVolume(0);
#endif
    FCEUI_SetSoundQuality(0);
    FCEUI_SetLowPass(0);

    if (Settings.sound >= SAMPLERATES_SIZE) {
        Settings.sound = 0;
    }
#ifndef SOUND_ON
    Settings.sound = 0;
#endif
    FCEUI_Sound(SND_sampleRates[Settings.sound]);
    
#ifdef SOUND_ON
    struct audsrv_fmt_t format;
    format.bits = 16;
    format.freq = SND_sampleRates[Settings.sound];
    format.channels = 1;
    audsrv_set_format(&format);
    audsrv_set_volume(MAX_VOLUME);
#endif
}

void SND_SetNextSampleRate()
{
#ifdef SOUND_ON
    Settings.sound++;
    SND_Init();
#endif
}

int SND_GetCurrSampleRate()
{
    return SND_sampleRates[Settings.sound];
}
