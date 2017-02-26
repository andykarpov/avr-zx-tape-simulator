#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


unsigned char readByte(FILE *f) {
	return fgetc(f);
}

unsigned short readWord(FILE *f) {
	return fgetc(f) + (fgetc(f) << 8);
}

void writeByte(FILE *f, unsigned int b) {
	fputc(b, f);
}

void writeWord(FILE *f, unsigned int b) {
	fputc(b & 0xff, f);
	fputc((b >> 8) & 0xff, f);
}

void writeDword(FILE *f, unsigned int b) {
	fputc(b & 0xff, f);
	fputc((b >> 8) & 0xff, f);
	fputc((b >> 16) & 0xff, f);
	fputc((b >> 24) & 0xff, f);
}


bool baw2wav(const char* srcName, const char *outName) {
	unsigned char *bawData = NULL;
	
	FILE* fin = fopen(srcName, "rb");
	if (!fin) {
		printf("file not found: %s\n", srcName);
		return false;
	}
	FILE* fout = fopen(outName, "wb");
	if (!fout) {
		printf("can't create file: %s\n", srcName);
		fclose(fin);
		return false;
	}

	fseek(fin, 0L, SEEK_END);
	unsigned int bawFileSize = ftell(fin);
	fseek(fin, 0L, SEEK_SET);

	printf("BAW file size %i\n", bawFileSize);

	if (fgetc(fin) != 'B' || fgetc(fin) != 'W' || fgetc(fin) != 'A' || fgetc(fin) != 'V') {
		printf("wrong signature\n");
		goto error;
	}

	int version = readWord(fin);
	printf("BAW version: %i\n", version);
	if (version != 1) {
		printf("Unknown version: %i\n", version);
		goto error;
	}

	int sampleRate = readWord(fin);
	printf("Sample rate: %i\n", sampleRate);

	// load data to memory
	unsigned int bawDataSize = bawFileSize - 8;
	bawData = malloc(bawDataSize);
	if (!bawData) {
		printf("Can't allocate %i bytes for read buffer\n", bawDataSize);
		goto error;
	}

	if (fread(bawData, 1, bawDataSize, fin) != bawDataSize) {
		printf("Can't read BAW data\n");
		goto error;
	}

	// calculate number of samples
	unsigned int samplesCount = 0;
	for (unsigned int i = 0; i < bawDataSize; i++) {
		unsigned char b = bawData[i];
		if (b == 0) {
			break;
		}
		samplesCount += b & 0b01111111;
	}
	printf("Samples: %i\n", samplesCount);
	printf("Duration: %.2f sec\n", 1.0*samplesCount/sampleRate);

	
	unsigned int chunkSize = samplesCount + 44 - 8;

	// write header
	writeDword(fout, 0x46464952);		// 0: 'RIFF'			[4]
	writeDword(fout, chunkSize);		// 4: chunk_size		[4]		= file_size - 8
	writeDword(fout, 0x45564157);		// 8: 'WAVE'			[4]
	writeDword(fout, 0x20746d66);		// 12: 'fmt '			[4]
	writeDword(fout, 0x10);			// 16: fmt_chunk_size	[4]		= 0x10
	writeWord(fout, 1);				// 20: audio_format		[2]		= 1 (PCM, no compression)
	writeWord(fout, 1);				// 22: num_channels		[2]		= 1 (mono)
	writeDword(fout, sampleRate);		// 24: sample_rate		[4]
	writeDword(fout, sampleRate);		// 28: byte_rate		[4]		= sample_rate   (== sample_rate * num_channels * bits_per_sample/8)
	writeWord(fout, 1);				// 32: block_align		[2]		= 1	(== num_channels * bits_per_sample/8)
	writeWord(fout, 8);				// 34: bits_per_sample	[2]		= 8
	writeDword(fout, 0x61746164);		// 36: 'data'			[4]
	writeDword(fout, samplesCount);	// 40: data_cunk_size	[4]		= file_size - 44

	for (unsigned int i = 0; i < bawDataSize; i++) {
		unsigned char b = bawData[i];
		if (b == 0) {
			break;
		}
		unsigned char level = b & 0b10000000 ? 0xff : 0x01;
		unsigned char cnt = b & 0b01111111;
		for (unsigned char j = 0; j < cnt; j++) {
			writeByte(fout, level);
		}
	}
	
	bool result = true;
error:
	result = false;
	if (bawData) {
		free(bawData);
	}
	fclose(fin);
	fclose(fout);
	return result;
}



int main(int argc, char* argv[]) {	
	if (argc != 3) {
		printf("usage: baw2wav <file.baw> <file.wav>\n");
		return -1;
	}
	baw2wav(argv[1], argv[2]);

	return 0;
}

