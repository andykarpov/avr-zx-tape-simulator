
#include <stdio.h>
#include <vector>

#define _CRT_SECURE_NO_WARNINGS	1

#define MINIMUM_BLOCKS_GAP_DURATION_MS		500



typedef struct Block {
	double duration;				// in ms
	unsigned int size;			// in bytes
	unsigned int offset;			// from end of header

	Block(unsigned int _offset) { offset = _offset; size = 0; duration = 0; };
} Block;


std::vector<Block*> blocks;

unsigned char readByte(FILE *f) {
	return fgetc(f);
}

unsigned short readWord(FILE *f) {
	return fgetc(f) + (fgetc(f) << 8);
}

unsigned int readDword(FILE *f) {
	unsigned int result = 0;
	fread(&result, 4, 1, f);
	return result;
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


bool readLevel(FILE *f) {
	return (readByte(f) & 0x80) == 0;
}

bool wav2baw(const char* srcName, const char *outName) {
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

	// read wav header
	if (readDword(fin) != 0x46464952) {
		printf("wrong wav signature, 'RIFF' expected\n");
		goto exit;
	}
	unsigned int chunksize = readDword(fin);
	if (readDword(fin) != 0x45564157) {
		printf("wrong wav signature, 'WAVE' expected\n");
		goto exit;
	}
	if (readDword(fin) != 0x20746d66) {
		printf("wrong wav signature, 'fmt ' expected\n");
		goto exit;
	}
	unsigned int fmtChunksize = readDword(fin);
	if (fmtChunksize != 0x10) {
		printf("wrong fmt_chunk_size'\n");
		goto exit;
	}
	unsigned int audioFormat = readWord(fin);
	if (audioFormat != 1) {
		printf("PCF format expected'\n");
		goto exit;
	}
	unsigned int numChannels = readWord(fin);
	if (numChannels != 1) {
		printf("mono file expected'\n");
		goto exit;
	}
	unsigned int sampleRate = readDword(fin);
	printf("Sample rate: %i\n", sampleRate);
	unsigned int byteRate = readDword(fin);
	if (byteRate != sampleRate) {
		printf("invalid byte_rate: %i'\n", byteRate);
		goto exit;
	}
	unsigned short blockAlign = readWord(fin);
	unsigned int bitPerSample = readWord(fin);
	if (readDword(fin) != 0x61746164) {
		printf("wrong wav signature, 'data' expected\n");
		goto exit;
	}
	unsigned int dataChunkSize = readDword(fin);
	

	writeByte(fout, 'B');
	writeByte(fout, 'W');
	writeByte(fout, 'A');
	writeByte(fout, 'V');

	writeWord(fout, 1);		// version
	writeWord(fout, sampleRate);

	bool level = readLevel(fin);
	int length = 1;

	const double dt = 1000.0 / sampleRate;		// in milliseconds
	double pauseDuration = 0;
	
	unsigned int pauseSize = 0;
	unsigned int offset = 0;

	Block *currentBlock = new Block(0);
	currentBlock->duration = dt;

	while (!feof(fin)) {
		bool newLevel = readLevel(fin);
		if (feof(fin)) {
			printf("LEVEL: %i %i \n", newLevel, level);
			break;
		}
		currentBlock->duration += dt;
		if (newLevel == level) {
			pauseDuration += dt;
			length++;
			if (length >= 0x7f) {
				writeByte(fout, length + (level ? 0x80 : 0));
				offset++;
				pauseSize++;
				length = 0;
			}
		} else {
			if (pauseDuration >= MINIMUM_BLOCKS_GAP_DURATION_MS) {
				currentBlock->size = offset - currentBlock->offset - pauseSize / 2;
				currentBlock->duration -= pauseDuration / 2;
				blocks.push_back(currentBlock);
				currentBlock = new Block(offset - pauseSize / 2);
				currentBlock->duration = pauseDuration / 2;
			}
			pauseDuration = 0;
			writeByte(fout, length + (level ? 0x80 : 0));
			offset++;
			pauseSize = 0;
			level = newLevel;
			length = 1;
		}
	}
	currentBlock->size = offset - currentBlock->offset;
	blocks.push_back(currentBlock);
	printf("pauseDuration = %f\n", (float)pauseDuration);

	writeByte(fout, 0);

	// write blocks descriptor
	for (int i = 0; i < blocks.size(); i++) {
		Block* b = blocks.at(i);
		writeDword(fout, b->offset);
		writeDword(fout, b->size);
		writeDword(fout, (int)b->duration);
		printf("BLOCK-%i %i\t%i\t\t%f\n", i+1, b->offset, b->size, (float)b->duration/1000);
	}
	writeByte(fout, blocks.size());

	fclose(fin);
	fclose(fout);

	for (int i = 0; i < blocks.size(); i++) {
		delete blocks.at(i);
	}

	return true;


exit:

	fclose(fin);
	fclose(fout);
	return false;
}

int main(int argc, char* argv[]) {
	
	if (argc != 3) {
		printf("usage: wav2baw <file.wav> <file.baw>\n");
		return -1;
	}
	wav2baw(argv[1], argv[2]);

	return 0;
}

