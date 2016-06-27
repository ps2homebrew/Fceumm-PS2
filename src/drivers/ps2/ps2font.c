#include <stdio.h>
#include <string.h>

#include <gsKit.h>
#include <dmaKit.h>

#include "ps2fceu.h"

extern vars Settings;
extern GSGLOBAL *gsGlobal;
u8        *FontBuffer;

extern unsigned char font_uLE[];
enum {
//0x100-0x109 are 5 double width characters for D-Pad buttons, which are accessed as:
//"ÿ0"==Circle  "ÿ1"==Cross  "ÿ2"==Square  "ÿ3"==Triangle  "ÿ4"==filled Square
	RIGHT_CUR = 0x10A, //Triangle pointing left, for use to the right of an item
	LEFT_CUR  = 0x10B, //Triangle pointing right, for use to the left of an item
	UP_ARROW  = 0x10C, //Arrow pointing up
	DN_ARROW  = 0x10D, //Arrow pointing up
	LT_ARROW  = 0x10E, //Arrow pointing up
	RT_ARROW  = 0x10F, //Arrow pointing up
	TEXT_CUR  = 0x110, //Vertical bar, for use between two text characters
	UL_ARROW  = 0x111, //Arrow pointing up and to the left, from a vertical start.
	BR_SPLIT  = 0x112, //Splits rectangle from BL to TR with BR portion filled
	BL_SPLIT  = 0x113, //Splits rectangle from TL to BR with BL portion filled
//0x114-0x11B are 4 double width characters for D-Pad buttons, which are accessed as:
//"ÿ:"==Right  "ÿ;"==Down  "ÿ<"==Left  "ÿ="==Up
	FONT_COUNT= 0x11C  //Total number of characters in font
};

/*GSTEXTURE FONT_TEX;

struct {
    int Rows;
    int Columns;
    int CharWidth;
    int CharHeight;
     u8 Additional[256];
} fontdata;

int font_init(GSGLOBAL *gsGlobal)
{
    int i;
    char *fontpath;
    char *datpath;
    fontpath = calloc(1,strlen(Settings.savepath)+8);
    datpath = calloc(1,strlen(Settings.savepath)+8);

    sprintf(fontpath,"%s%s",Settings.savepath,"strt.png");
    sprintf(datpath,"%s%s",Settings.savepath,"strt.dat");

    if( gsKit_texture_png(gsGlobal, &FONT_TEX, fontpath) == -1 ) {
        printf("Error uploading font png!\n");
        return -1;
    }

    fontdata.Columns=16;
    fontdata.Rows=16;
    fontdata.CharWidth = FONT_TEX.Width / 16;
    fontdata.CharHeight = FONT_TEX.Height / 16;

    int File = fioOpen(datpath, O_RDONLY);
    //fontdata.Additional=malloc( 0x100 );
    u16 temp_buffer[512];
    if (File > 0) {
        fioLseek(File, 0, SEEK_SET);
        if(fioRead(File, &temp_buffer, 0x200) <= 0) {
            printf("Could not load font sizes: %s\n", datpath);
            return -1;
        }
        fioClose(File);
        for (i = 0; i < 0x100; i++) {
            fontdata.Additional[i] = temp_buffer[i];
        }
    }
    else {
        int i;
        for (i = 0; i < 0x100; i++) {
            fontdata.Additional[i] = fontdata.CharWidth;
        }
    }

    return 0;
}

void font_print(GSGLOBAL *gsGlobal, float X, float Y, int Z, unsigned long color, char *String)
{
    u64 oldalpha = gsGlobal->PrimAlpha;
    u8 oldpabe = gsGlobal->PABE;

    u8 fixate = 0;

    //u64 oldalpha = gsGlobal->PrimAlpha;
    //gsGlobal->PrimAlpha=ALPHA_BLEND_ADD;
    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);

    if(gsGlobal->Test->ATE) {
        gsKit_set_test(gsGlobal, GS_ATEST_OFF);
        fixate = 1;
    }

    int cx,cy,i,l;
    char c;
    cx=X;
    cy=Y;
    l=strlen( String );
    for( i=0;i<l;i++ ) {

        c=String[i];
        if( c=='\n' ) {
            cx=X;
            cy+=fontdata.CharHeight;
        }
        else {

            int px,py,charsiz;
            px=c%16;
            py=(c-px)/16;
            charsiz=fontdata.Additional[(u8)c];

            if(gsGlobal->Interlace == GS_INTERLACED) {
                gsKit_prim_sprite_texture(gsGlobal, &FONT_TEX, cx, cy,
                    px*fontdata.CharWidth+1, py*fontdata.CharHeight+1,
                    cx+fontdata.CharWidth, cy+fontdata.CharHeight,
                    (px+1)*fontdata.CharWidth, (py+1)*fontdata.CharHeight,
                    Z, color);
            }
            if(gsGlobal->Interlace == GS_NONINTERLACED) {
                gsKit_prim_sprite_texture(gsGlobal, &FONT_TEX, cx, cy,
                    px*fontdata.CharWidth+1, py*fontdata.CharHeight,
                    cx+fontdata.CharWidth, cy+fontdata.CharHeight,
                    (px+1)*fontdata.CharWidth, (py+1)*fontdata.CharHeight-1,
                    Z, color);
            }
            cx+=charsiz+1;
        }
    }
    if(fixate)
        gsKit_set_test(gsGlobal, GS_ATEST_ON);

    gsGlobal->PABE = oldpabe;
    gsGlobal->PrimAlpha=oldalpha;
    // gsKit_set_primalpha(gsGlobal, gsGlobal->PrimAlpha, gsGlobal->PABE);
    //gsKit_set_primalpha(gsGlobal, gsGlobal->PrimAlpha, gsGlobal->PABE);
}*/

int loadFont(char *path_arg)
{
/*	int fd;

	if(strlen(path_arg) != 0 ){
	char FntPath[1025];
	genFixPath(path_arg, FntPath);
	fd = genOpen( FntPath, O_RDONLY );
		if(fd < 0){
			genClose( fd );
			goto use_default;
		} // end if failed open file
		genLseek( fd, 0, SEEK_SET );
		if(genLseek( fd, 0, SEEK_END ) > 4700){
			genClose( fd );
			goto use_default;
		}
		genLseek( fd, 0, SEEK_SET );
		u8 FontHeader[100];
		genRead( fd, FontHeader, 100 );
		if((FontHeader[ 0]==0x00) &&
		   (FontHeader[ 1]==0x02) &&
		   (FontHeader[70]==0x60) &&
		   (FontHeader[72]==0x60) &&
		   (FontHeader[83]==0x90)){
			genLseek( fd, 1018, SEEK_SET );
			if(FontBuffer)
				free(FontBuffer);
			FontBuffer = malloc( 4096 + 1 );
			genRead( fd, FontBuffer+32*16, 3584 ); // First 32 Chars Are Not Present In .fnt Files
			genClose( fd );
			free(FontHeader);
			return 1;
		}else{ // end if good fnt file
			genClose( fd );
			free(FontHeader);
			goto use_default;
		} // end else bad fnt file
	}else{ // end if external font file*/
//use_default:
		if(FontBuffer)
			free(FontBuffer);
		FontBuffer = malloc( 4096 + 1 );
		memcpy( FontBuffer, &font_uLE, 4096 );
	//} // end else build-in font
	return 0;
}

void drawChar(unsigned int c, int x, int y, int z, u64 colour)
{
	int i, j, pixBase, pixMask;
	u8  *cm;

	if(!Settings.interlace){
		y = y & -2;
	}

	if(c >= FONT_COUNT) c = '_';
	if(c > 0xFF)              //if char is beyond normal ascii range
		cm = &font_uLE[c*16];   //  cm points to special char def in default font
	else                      //else char is inside normal ascii range
		cm = &FontBuffer[c*16]; //  cm points to normal char def in active font

	pixMask = 0x80;
	for(i=0; i<8; i++){	//for i == each pixel column
		pixBase = -1;
		for(j=0; j<16; j++){ //for j == each pixel row
			if((pixBase < 0) && (cm[j] & pixMask)){ //if start of sequence
				pixBase = j;
			} else if((pixBase > -1) && !(cm[j] & pixMask)){ //if end of sequence
				gsKit_prim_sprite(gsGlobal, x+i, y+pixBase-1, x+i+1, y+j-1, z, colour);
				pixBase = -1;
			}
		}//ends for j == each pixel row
		if(pixBase > -1) //if end of sequence including final row
			gsKit_prim_sprite(gsGlobal, x+i, y+pixBase-1, x+i+1, y+j-1, z, colour);
		pixMask >>= 1;
	}//ends for i == each pixel column
}

int printXY(const char *s, int x, int y, int z, u64 colour, int draw, int space)
{
	unsigned int c1, c2;
	int i;
	int text_spacing=8;

	if(space>0){
		while((strlen(s)*text_spacing) > space)
			if(--text_spacing<=5)
				break;
	}else{
		while((strlen(s)*text_spacing) > gsGlobal->Width-6-8*2)
			if(--text_spacing<=5)
				break;
	}

	i=0;
	while((c1=s[i++])!=0) {
		if(c1 != 0xFF) { // Normal character
			if(draw) drawChar(c1, x, y, z, colour);
			x += text_spacing;
			if(x > gsGlobal->Width-6-8)
				break;
			continue;
		}  //End if for normal character
		// Here we got a sequence starting with 0xFF ('ÿ')
		if((c2=s[i++])==0)
			break;
		if((c2 < '0') || (c2 > '='))
			continue;
		c1=(c2-'0')*2+0x100;
		if(draw) {
			//expand sequence ÿ0=Circle  ÿ1=Cross  ÿ2=Square  ÿ3=Triangle  ÿ4=FilledBox
			//"ÿ:"=Pad_Right  "ÿ;"=Pad_Down  "ÿ<"=Pad_Left  "ÿ="=Pad_Up
			drawChar(c1, x, y, z, colour);
			x += 8;
			if(x > gsGlobal->Width-6-8)
				break;
			drawChar(c1+1, x, y, z, colour);
			x += 8;
			if(x >gsGlobal->Width-6-8)
				break;
		}
	}  // ends while(1)
	return x;
}
