#ifdef SOUND_HQ
int32 NeoFilterSound(int32 *in, int32 *out, uint32 inlen, int32 *leftover);
#endif
void MakeFilters(int32 rate);
void SexyFilterSetup();
void SexyFilter(int32 *in, int16 *out, int32 count);
