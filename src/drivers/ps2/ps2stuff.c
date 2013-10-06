#include <stdio.h>
#include "../../types.h"
#include "../../driver.h"

struct pcpal {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} pcpalette[256];
extern unsigned int ps2palette[256];

//main
FILE *FCEUD_UTF8fopen(const char *fn, const char *mode)
{
	return(fopen(fn,mode));
}

//Functions to display messages differently
void FCEUD_PrintError(char *s)
{
	printf("Error %s", s);
}

void FCEUD_Message(char *s)
{
	printf("%s", s);
}

void FCEUD_SetEmulationSpeed(int cmd)
{
	printf("SetEmulationSpeed not implemented.\n");
}

//Sound related functions
//could be implemented, but no real reason to
static int soundvolume=1024;
void FCEUD_SoundVolumeAdjust(int adjust)
{
	switch(adjust)
	{
	case -1:  soundvolume-=50; if(soundvolume<0) soundvolume=0; break; //lower volume
	case 0:	  soundvolume=1024; break;
	case 1:	  soundvolume+=50; if(soundvolume>1024) soundvolume=1024; break; //raise volume
	}
	FCEUI_SetSoundVolume(soundvolume);
	FCEU_DispMessage("Sound volume %d.", soundvolume);
}

int mute = 0;
void FCEUD_SoundToggle(void)
{
	if(mute)
	{
		mute=0;
		FCEUI_SetSoundVolume(soundvolume);
		FCEU_DispMessage("Sound mute off.");
	}
	else
	{
		mute=1;
		FCEUI_SetSoundVolume(0);
		FCEU_DispMessage("Sound mute on.");
	}
}

//Network
int FCEUD_SendData(void *data, uint32 len)
{
	printf("Send Network Data");
	return(0);
}

void FCEUD_NetworkClose(void)
{
	printf("Close Network");
}

int FCEUD_RecvData(void *data, uint32 len)
{
	printf("Receive Network Data");
	return(0);
}

void FCEUD_NetplayText(uint8 *text)
{
	printf("Network Text");
}

//Video
void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b)
{
	pcpalette[index].r = r;
	pcpalette[index].g = g;
	pcpalette[index].b = b;
}

void FCEUD_GetPalette(uint8 i,uint8 *r, uint8 *g, uint8 *b)
{
	*r = pcpalette[i].r;
	*g = pcpalette[i].g;
	*b = pcpalette[i].b;
}


#define DUMMY(f) void f(void) {FCEU_DispMessage("Not implemented.");}
//#define DUMMY(f) void f(void) {;}; //was getting the message above, but seems to have disappeared
DUMMY(FCEUD_HideMenuToggle)
DUMMY(FCEUD_TurboOn)
DUMMY(FCEUD_TurboOff)
DUMMY(FCEUD_SaveStateAs)
DUMMY(FCEUD_LoadStateFrom)
DUMMY(FCEUD_MovieRecordTo)
DUMMY(FCEUD_MovieReplayFrom)
DUMMY(FCEUD_ToggleStatusIcon)
DUMMY(FCEUD_AviRecordTo)
DUMMY(FCEUD_AviStop)
void FCEUI_AviVideoUpdate(const unsigned char* buffer) {/*FCEU_DispMessage("Not implemented.");*/}
int FCEUD_ShowStatusIcon(void) {/*FCEU_DispMessage("Not implemented.");*/ return 0; }
int FCEUI_AviIsRecording(void) {return 0;}
