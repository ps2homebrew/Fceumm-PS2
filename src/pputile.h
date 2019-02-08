uint8 *C;
uint8 cc;
uint32 vadr;

if (X1 >= 2) {
	uint8 *S = PALRAM;
	uint32 pixdata;

	pixdata = ppulut1((pshift[0] >> (8 - XOffset)) & 0xFF) | ppulut2((pshift[1] >> (8 - XOffset)) & 0xFF);

	pixdata |= ppulut3[XOffset | (atlatch << 3)];

	P[0] = S[pixdata >> 0 & 0xF];
	P[1] = S[pixdata >> 4 & 0xF];
	P[2] = S[pixdata >> 8 & 0xF];
	P[3] = S[pixdata >> 12 & 0xF];
	P[4] = S[pixdata >> 16 & 0xF];
	P[5] = S[pixdata >> 20 & 0xF];
	P[6] = S[pixdata >> 24 & 0xF];
	P[7] = S[pixdata >> 28 & 0xF];
	P += 8;
}

#ifdef PPUT_MMC5SP
    uint8 xs = X1;
	vadr = (MMC5HackExNTARAMPtr[xs | (ys << 5)] << 4) + (vofs & 7);
#else
	C = vnapage[(RefreshAddr >> 10) & 3];
	vadr = (C[RefreshAddr & 0x3ff] << 4) + vofs;	// Fetch name table byte.
#endif

#ifdef PPUT_HOOK
	PPU_hook(0x2000 | (RefreshAddr & 0xfff));
#endif

#ifdef PPUT_MMC5SP
	cc = MMC5HackExNTARAMPtr[0x3c0 + (xs >> 2) + ((ys & 0x1C) << 1)];
	cc = ((cc >> ((xs & 2) + ((ys & 0x2) << 1))) & 3);
#else
	#ifdef PPUT_MMC5CHR1
		cc = (MMC5HackExNTARAMPtr[RefreshAddr & 0x3ff] & 0xC0) >> 6;
	#else
	    uint8 zz = RefreshAddr & 0x1F;
		cc = C[0x3c0 + (zz >> 2) + ((RefreshAddr & 0x380) >> 4)];	// Fetch attribute table byte.
		cc = ((cc >> ((zz & 2) + ((RefreshAddr & 0x40) >> 4))) & 3);
	#endif
#endif

atlatch >>= 2;
atlatch |= cc << 2;


#ifdef PPUT_MMC5SP
	C = MMC5HackVROMPTR + vadr;
	C += ((MMC5HackSPPage & 0x3f & MMC5HackVROMMask) << 12);
#else
	#ifdef PPUT_MMC5CHR1
		C = MMC5HackVROMPTR;
		C += (((MMC5HackExNTARAMPtr[RefreshAddr & 0x3ff]) & 0x3f & MMC5HackVROMMask) << 12) + (vadr & 0xfff);
	#elif defined(PPUT_MMC5)
		C = MMC5BGVRAMADR(vadr);
	#else
		C = VRAMADR(vadr);
	#endif
#endif

#ifdef PPUT_HOOK
	PPU_hook(vadr);
#endif

#ifdef PPU_BGFETCH
	if (RefreshAddr & 1) {
        pshift[0] = pshift[0] << 8 | C[8];
        pshift[1] = pshift[1] << 8 | C[8];
	} else {
        pshift[0] = pshift[0] << 8 | C[0];
        pshift[1] = pshift[1] << 8 | C[0];
	}
#else
    pshift[0] = pshift[0] << 8 | C[0];
    pshift[1] = pshift[1] << 8 | C[8];
#endif

if ((RefreshAddr & 0x1f) == 0x1f)
	RefreshAddr ^= 0x41F;
else
	RefreshAddr++;

#ifdef PPUT_HOOK
	PPU_hook(0x2000 | (RefreshAddr & 0xfff));
#endif

