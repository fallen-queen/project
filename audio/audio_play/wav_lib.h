#ifndef __WAV_LIB__
#define __WAV_LIB__

/**************************************************************************/
/*******************************DEFINITIONS********************************/
#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

/**************************************************************************/
/***********************************TYPES**********************************/
typedef int bool;

typedef struct _RIFF_HEADER {
	unsigned char  RiffID[4];
	unsigned int   RiffSize;
	unsigned char  RiffFormat[4];
} RIFF_HEADER;

typedef struct _WAVE_FORMAT {
	unsigned short FormatTag;
	unsigned short Channels;
	unsigned int   SamplesPerSec;
	unsigned int   AvgBytesPerSec;
	unsigned short BlockAlign;
	unsigned short BitsPerSample;
} WAVE_FORMAT;

typedef struct _FMT_BLOCK {
	unsigned char  FmtID[4];
	unsigned int   FmtSize;
	WAVE_FORMAT    wavFormat;
} FMT_BLOCK;

typedef struct _DATA_BLOCK {
	unsigned char  DataID[4];
	unsigned int   DataSize;
} DATA_BLOCK;

typedef struct _WAVE_HEADER {
	RIFF_HEADER  RiffHdr;
	FMT_BLOCK    FmtBlk;
	DATA_BLOCK   DataBlk;
} WAVE_HEADER;

/**************************************************************************/
/***************************LOCAL FUNCTIONS********************************/

/**************************************************************************/
/*************************GLOBAL FUNCTIONS*********************************/
bool parse_wav_header(WAVE_HEADER *wavHdr);

bool read_wav_file(char *file_name, void *data, unsigned int length, int offset);

#endif
