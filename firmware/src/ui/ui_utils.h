/*
 * ui_utils.h
 *
 * Created: 05.07.2015 21:21:22
 *  Author: Trol
 */ 


#ifndef _UI_UTILS_H_
#define _UI_UTILS_H_


#define LINES_PER_SCREEN			5	// ����� �� ������
#define LINES_DY						9	// ���������� ����� ��������

#define MAX_FILE_NAME_DISPLAY_LEN	12	// ������������ ������������ ����� ����� �����





uint8_t screen = SCREEN_MAIN_MENU;

// ����� ������� �������� �� ������ ����� �� ������ sdcard_ls_data
uint16_t firstVisibleItem;

// ����� ����������� �������� �� ������ (�� ������������)
uint8_t selectedIndex;

// ����� �������������� �������� � ����������
bool editMode;
// ������������� ��������
uint16_t editedValue;


// ���������� � TAP-����� ����� ��� ��������
struct tap_checker_out_struct tapInfo;

// ������������ �������������� WAV/BAW ����� � ������� ����� �������
uint16_t playedFileDuration;

char currentFileName[MAX_FILE_NAME_DISPLAY_LEN + 5];	// ��� �������� ����� (������������ ��� ������������)
uint32_t fileSize;										// ������ �������� ������������ ��� ������������ �����

static char* browserGetSelectedFile();

static void refreshBrowser();
static void refreshBrowserFrom(uint32_t offset);
static bool gotoBack();


// TODO !!! ���������� ��������� ���� �������� ��������� �� ������ settings !!!

// ������� �������������� ���������� ���������
const uint16_t MIN_TAP_TIMING_VALUES[7] PROGMEM = {
	500,		// ����� ����� �������, ��
	500,		// ����� ����� ���������� � ������, ��
	500,		// ��������� ����� ������, ��
	50,		// ����� ��������� ������, ���
	50,		// ����� ��������� ������-1, ���
	50,		// ����� ��������� ������-2, ���
	50,		// ����� ��������� ������, ���
};

const uint16_t MAX_TAP_TIMING_VALUES[7] PROGMEM = {
	10000,	// ����� ����� �������, ��
	10000,	// ����� ����� ���������� � ������, ��
	5000,		// ��������� ����� ������, ��
	2000,		// ����� ��������� ������, ���
	1000,		// ����� ��������� ������-1, ���
	1000,		// ����� ��������� ������-2, ���
	1000,		// ����� ��������� ������, ���
};


const uint16_t MIN_BAW_TIMING_VALUES[4] PROGMEM = {
	0,			// sample rate index
	100,		// ����������� ����� ����� �������
	0,			// ��������
	1000		// ����� ���������
};


const uint16_t MAX_BAW_TIMING_VALUES[4] PROGMEM = {
	5,			// sample rate index
	3000,		// ����������� ����� ����� �������
	0,			// ��������
	20000		// ����� ���������
};

const uint8_t BAW_TIMINGS_DELTA[4] PROGMEM = {
	1,
	10,
	0,
	100
};


const uint16_t MIN_RKR_TIMING_VALUES[2] PROGMEM = {
	130,			// baudrate
	0				// padding
};
const uint16_t MAX_RKR_TIMING_VALUES[2] PROGMEM = {
	60000,		// baudrate
	500,			// padding
};
const uint8_t RKR_TIMINGS_DELTA[2] PROGMEM = {
	10,
	5,
};


/************************************************************************/
/* ��������������� �������                                              */
/************************************************************************/

// ������ ����� �� ������
static uint8_t glcd_draw_uint16(uint8_t x, uint8_t y, uint16_t val) {
	uint16_t num = 10000;
	bool started = false;
	uint8_t result = 0;
	while (num > 0) {
		uint8_t b = val / num;
		if (b > 0 || started || num == 1) {
			result += glcd_draw_char_xy(x+result, y, '0' + b) + 1;
			started = true;
		}
		val -= b * num;
		num /= 10;
	}
	return result;
}


static uint8_t glcd_draw_uint32(uint8_t x, uint8_t y, uint32_t val) {
	uint32_t num = 10000000;
	bool started = false;
	uint8_t result = 0;
	while (num > 0) {
		uint8_t b = val / num;
		if (b > 0 || started || num == 1) {
			result += glcd_draw_char_xy(x+result, y, '0' + b) + 1;
			started = true;
		}
		val -= b * num;
		num /= 10;
	}
	return result;
}


// ���������� ����� ���������� ������������� ����� � ��������
static uint8_t glcd_getUint16digits(uint16_t val) {
	uint16_t num = 10000;
	bool started = false;
	uint8_t result = 0;
	while (num > 0) {
		uint8_t b = val / num;
		if (b > 0 || started || num == 1) {
			result ++;
			started = true;
		}
		val -= b * num;
		num /= 10;
	}
	return result;
}

// ������ ���������� ����� (������������ ��� ������ ���� � �������)
static uint8_t glcd_draw_uint8_2(uint8_t x, uint8_t y, uint8_t val) {
	uint8_t result = 0;
	result += glcd_draw_char_xy(x, y, '0' + (val / 10)) + 1;
	x += result;
	result += glcd_draw_char_xy(x, y, '0' + (val % 10)) + 1;

	return result;
}

// ������ ������, ���������������� �� �����������
// ������� � ����� 1..10 ����������� ������
static void glcd_drawCenteredStr_p(const char *str, uint8_t y, uint8_t dx) {
	uint8_t len = 0;//strlen_P(str);
	const char *str0 = str;
	while (1) {
		uint8_t c = pgm_read_byte(str0++);
		if (!c) {
			break;
		}
		if (c <= 10) {
			len += c;
//		} else if (c <= 15) {
//			len -= (c - 10);
		} else {
			len += 5 + dx;
		}
	}
	//uint8_t x = (GLCD_LCD_WIDTH - len*5 - (len-1)*dx)/2;
	uint8_t x = (GLCD_LCD_WIDTH - len)/2;
	while (1) {
		uint8_t c = pgm_read_byte(str++);
		if (!c) {
			return;
		}
		if (c <= 10) {
			x += c;
//		} else if (c <= 15) {
//			x -= (c - 10);
		} else {
			x += glcd_draw_char_xy(x, y, c) + dx;
		}
	}
}

// ������ ��������� �������� ���������������� �� ����������� ���� ��� ������
// �������� ���������� �������� ������ \n
static void glcd_drawCenterStrings_p(const char* str, uint8_t y, uint8_t dx) {
//	uint8_t len = strlen_P(str);
	while (1) {
		// ��������� ������ ��������� ��������� � �� ������
		uint8_t len = 0;
		
		char ch;
		do {
			ch = pgm_read_byte(str++);
			len++;
		} while (ch != '\n' && ch != 0);

		uint8_t x = (GLCD_LCD_WIDTH - len*5 - (len-2)*dx)/2;
		str -= len;

		for (uint8_t i = 0; i < len; i++) {
			ch = pgm_read_byte(str++);
			if (ch == 0) {
				return;
			} else if (ch == '\n') {
				continue;
			}
			x += glcd_draw_char_xy(x, y, ch) + dx;
		}
		y += LINES_DY;
	}
}


static void glcd_drawCenteredStr(const char *str, uint8_t y, uint8_t dx) {
	uint8_t len = strlen(str);
	uint8_t x;
	if (len <= 15) {
		x = (GLCD_LCD_WIDTH - len*5 - (len-1)*dx)/2;
		} else {
		x = 0;
	}
	uint8_t i = 0;
	while (1) {
		char c = str[i++];
		if (!c) {
			return;
		}
		x += glcd_draw_char_xy(x, y, c) + dx;
		c++;
	}
}

static uint8_t glcd_drawHexByte(uint8_t val, uint8_t x, uint8_t y, uint8_t dx) {
	uint8_t hi = val >> 4;
	uint8_t lo = val & 0x0f;
	
	x += glcd_draw_char_xy(x, y, hi < 0x0a ? hi + '0' : hi - 0x0a + 'A') + dx;
	x += glcd_draw_char_xy(x, y, lo < 0x0a ? lo + '0' : lo - 0x0a + 'A') + dx;
	
	return x;
}

static uint8_t glcd_drawHexWord(uint16_t val, uint8_t x, uint8_t y, uint8_t dx) {
	x = glcd_drawHexByte((uint8_t)(val >> 8), x, y, dx) + dx;
	return glcd_drawHexByte((uint8_t)(val & 0xff), x, y, dx) + dx;
}



static void drawLeftKey(const char* str) {
	glcd_draw_string_xy_P(0, GLCD_LCD_HEIGHT-LINES_DY, str);
}


static void drawRightKey(const char *str) {
	uint8_t x = GLCD_LCD_WIDTH - strlen_P(str)*6;
	glcd_draw_string_xy_P(x, GLCD_LCD_HEIGHT-LINES_DY, str);
}

/************************************************************************/
/* ���������� ��� ����� (FILE_xxx)                                      */
/************************************************************************/
static uint8_t checkFileExt(const char* fileName) {
	uint8_t len = strlen(fileName);
	if (len < 4 || (fileName[len-4] != '.' && fileName[len-3] != '.')) {
		return FILE_UNKNOWN;
	}
	char ch1 = fileName[len-3] | 0x20;
	char ch2 = fileName[len-2] | 0x20;
	char ch3 = fileName[len-1] | 0x20;

	if (ch1 == 't' && ch2 == 'a' && ch3 == 'p') {
		return FILE_TAP;
	}
	if (ch1 == 'w' && ch2 == 'a' && ch3 == 'v') {
		return FILE_WAV;
	}
	if (ch1 == 'b' && ch2 == 'a' && ch3 == 'w') {
		return FILE_BAW;
	}
	if (ch1 == 'r'  && ch2 == 'k' && ch3 == 'r') {
		return FILE_RKR;
	}
	if (ch1 == '.' && ch2 == 'r' && ch3 == 'k') {
		return FILE_RKR;
	}
	if (ch1 == 'g' && ch2 == 'a' && ch3 == 'm') {
		return FILE_RKR;
	}
	return FILE_UNKNOWN;
}



#endif // _UI_UTILS_H_