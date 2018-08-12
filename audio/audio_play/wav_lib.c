#include "alsa_ctrl.h"
#include "wav_lib.h"

/**************************************************************************/
/*******************************DEFINITIONS********************************/
#define RIFF_ID      "RIFF"
#define RIFF_FORMAT  "WAVE"
#define FMT_ID       "fmt "
#define DATA_ID      "data"

/**************************************************************************/
/***********************************TYPES**********************************/

/**************************************************************************/
/***************************LOCAL FUNCTIONS********************************/

/**************************************************************************/
/*************************GLOBAL FUNCTIONS*********************************/
bool parse_wav_header(WAVE_HEADER *wavHdr)
{
	bool ret = false;
	RIFF_HEADER *RiffHdr = &wavHdr->RiffHdr;
	FMT_BLOCK *FmtBlk = &wavHdr->FmtBlk;
	WAVE_FORMAT *wavFormat = &FmtBlk->wavFormat;
	DATA_BLOCK *DataBlk = &wavHdr->DataBlk;

	if (!strcmp(RiffHdr->RiffID, RIFF_ID)) {
		printf("wrong RIFF ID\n");
		goto err_leave;
	}

	if (!strcmp(RiffHdr->RiffFormat, RIFF_FORMAT)) {
		printf("wrong RIFF format\n");
		goto err_leave;
	}

	if (!strcmp(FmtBlk->FmtID, FMT_ID)) {
		printf("wrong FMT ID\n");
		goto err_leave;
	}
	if (wavFormat->BitsPerSample != 16) {
		printf("invalid BitsPerSample\n");
		goto err_leave;
	}

	if (strcmp(DataBlk->DataID, DATA_ID)) {
		printf("wrong Data ID\n");
		goto err_leave;
	}

	ret = true;

err_leave:
	return ret;
}

bool read_wav_file(char *file_name, void *data, unsigned int length, int offset)
{
	bool ret = false;
	FILE *fp;

	if (!file_name || !*file_name || !data) {
		printf("invalid input parameters\n");
		goto err_leave;
	}

	if ((fp = fopen(file_name, "rb")) == NULL) {
		printf("Fail to open file %s\n", file_name);
		goto err_leave;
	}

	fseek(fp, offset, SEEK_SET);

	if (fread(data, 1, length, fp) != length) {
		printf("Fail to read file %s\n", file_name);
		goto err_leave;
	}

	ret = true;

err_leave:
	fclose(fp);
	return ret;
}

