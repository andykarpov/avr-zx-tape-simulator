#include "display.h"
#include "../debug.h"

#include "portaudio.h"
#include "../general.h"


#define KEY_MASK_LEFT	1
#define KEY_MASK_RIGHT	2
#define KEY_MASK_UP		4
#define KEY_MASK_DOWN	8
#define KEY_MASK_ENTER	16

#define ZOOM			4


#define get_highlight (!PORTA & _BV(0))

/**
 *  Screen buffer
 *
 *  Requires at least one bit for every pixel (e.g 504 bytes for 48x84 LCD)
 */
uint8_t glcd_buffer[GLCD_LCD_WIDTH * GLCD_LCD_HEIGHT / 8];

/**
 * Keeps track of bounding box of area on LCD which need to be
 * updated next refresh cycle
 */
glcd_BoundingBox_t glcd_bbox;


uint8_t key_pressed_mask = 0;
uint8_t key_mask = 0;

SDL_Window * window;
SDL_Renderer *ren;

SDL_TimerID timer2comp;
SDL_TimerID timer1ms;

PaStream *audio_stream;


char simulator_cwd[1024];
bool simulator_cwd_done = false;
uint8_t display_contrast = 70;
bool display_simulator_highlight = false;

void SimulatorInit();
void SimulatorDone();
static void display_check_hardware();





void glcd_init() {
	MSG("glcd_init");
	glcd_select_screen(glcd_buffer, &glcd_bbox);
	
	/* Clear screen, we are now ready to go */
	glcd_clear();
	
	glcd_reset_bbox();
	
	SimulatorInit();
	MSG("glcd_init-END");
}

void glcd_set_contrast(uint8_t contrast) {
	display_contrast = contrast;
	glcd_write();
}

void glcd_write() {
	display_check_hardware();
//	MSG("glcd_write");
	
	SDL_SetRenderDrawColor(ren, 0x80, 0x67, 0x48, 255);
	SDL_RenderClear(ren);

	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
	SDL_Rect r;
	r.w = ZOOM;
	r.h = ZOOM;
	uint8_t alpha = 255 - abs(display_contrast-80)*3;
	for (uint8_t x = 0; x < GLCD_LCD_WIDTH; x++) {
		for (uint8_t y = 0; y < GLCD_LCD_HEIGHT; y++) {
			uint8_t color = glcd_buffer_selected[x+ (y/8)*GLCD_LCD_WIDTH] & ( 1 << (y%8));
			
			if (color) {
				SDL_SetRenderDrawColor(ren, 0x25, 0x25, 0x1d, alpha);
			} else {
				if (display_simulator_highlight) {
					SDL_SetRenderDrawColor(ren, 0x95, 0xA2, 0xC4, alpha);
				} else {
					SDL_SetRenderDrawColor(ren, 0x65, 0x72, 0x54, alpha);
				}
			}
			//SDL_RenderDrawPoint(ren, x, y);
			r.x = x*ZOOM + 4;
			r.y = y*ZOOM + 4;
			SDL_RenderFillRect(ren, &r);
		}
	}
	SDL_RenderPresent(ren);
}


uint8_t getKeyMask(SDL_Event event) {
	switch (event.key.keysym.sym) {
		case SDLK_LEFT:
			return KEY_MASK_LEFT;
		case SDLK_RIGHT:
			return KEY_MASK_RIGHT;
		case SDLK_UP:
			return KEY_MASK_UP;
		case SDLK_DOWN:
			return KEY_MASK_DOWN;
		case SDLK_RETURN:
		case SDLK_RETURN2:
			return KEY_MASK_ENTER;
	}
	return 0;
}


uint8_t checkKbd() {
//	usleep(1);
	SDL_Event event;
	uint8_t mask;
	if (!SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				SimulatorDone();
				exit(0);
			case SDL_KEYDOWN:
				//printf( "Key press detected\n" );
				key_pressed_mask |= getKeyMask(event);
				break;
			case SDL_KEYUP:
				mask = getKeyMask(event);
				key_pressed_mask &= ~mask;
				break;	    		
		}
	}
	return 0;
}



bool key_down_pressed() {
	checkKbd();
	bool result = key_pressed_mask & KEY_MASK_DOWN;
	return result;
}

bool key_up_pressed() {
	checkKbd();
	bool result = key_pressed_mask & KEY_MASK_UP;
	return result;
}

bool key_left_pressed() {
	checkKbd();
	bool result = key_pressed_mask & KEY_MASK_LEFT;
	return result;
}

bool key_right_pressed() {
	checkKbd();
	bool result = key_pressed_mask & KEY_MASK_RIGHT;
	return result;
}

bool key_enter_pressed() {
	checkKbd();
	bool result = key_pressed_mask & KEY_MASK_ENTER;
	return result;
}

bool key_any_pressed() {
	checkKbd();
	return key_pressed_mask;
}

Uint32 timer2comp_callback(Uint32 interval, void *param) {
	bool hl = display_simulator_highlight;
	display_check_hardware();
	if (display_simulator_highlight != hl) {
		glcd_write();
	}
	TIMER2_COMP_vect();
	return interval;
}

Uint32 timer1ms_callback(Uint32 interval, void *param) {
	// прерывание по переполнению таймера 0 (таймер T0 8-битный и считает на увеличение до 0xff)
	if (TIMSK & _BV(TOIE0)) {
		TIMER0_OVF_vect();
	}
	if (TIMSK & _BV(OCIE1A)) {
		timer1compHandler();
	}
	return interval;
}

static int paCallback(const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, 
		PaStreamCallbackFlags statusFlags, void *userData) {

	float *out = (float*)outputBuffer;
	float v = OCR3BL ? 1.0 : 0;
	*out++ = v;
	*out++ = v;
	
	return paContinue;
}

void SimulatorInit() {
	initCwd();
	fprintf(stdout, "Current working dir: %s\n", simulator_cwd);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "\nUnable to initialize SDL:  %s\n", SDL_GetError());
		return;
	}
	
	int width = GLCD_LCD_WIDTH * ZOOM + 8;
	int height = GLCD_LCD_HEIGHT * ZOOM + 8;
	window = SDL_CreateWindow("Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	if (!window) {
		fprintf(stderr, "\nSDL_CreateWindow Error:  %s\n", SDL_GetError());
		return;
	}

	ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!ren) {
		fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
		return;
	}
	
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
	SDL_RenderClear(ren);
	
	timer2comp = SDL_AddTimer(10, timer2comp_callback, NULL);
	
	timer1ms = SDL_AddTimer(1, timer1ms_callback, NULL);
	
	PaError err = Pa_Initialize();
	if (err != paNoError) {
		printf("Error: can't initialize PortAudio");
	}
	
	
	// ----------------------------------------------------------------------
	PaStreamParameters outputParameters;
		
	outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device
	if (outputParameters.device == paNoDevice) {
      printf("Error: No default output device.\n");
		return;
	}
	outputParameters.channelCount = 2;       // stereo output
	outputParameters.sampleFormat = paFloat32; // 32 bit floating point output
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

// TODO задолбал писк при запуске
//	err = Pa_OpenStream(
//              &audio_stream,
//              NULL, // no input
//              &outputParameters,
//              44100,
//              2,	// FRAMES_PER_BUFFER,
//              paClipOff,      // we won't output out of range samples so don't bother clipping them
//              paCallback,
//              NULL);
//	
//	if (err != paNoError) {
//		printf("Error: can't open stream\n");
//		return;
//	}
//	err = Pa_StartStream(audio_stream);
//	if (err != paNoError) {
//		 printf("Error: can't start stream\n");
//		 return;
//	}
}

void SimulatorDone() {	
	PaError err = Pa_StopStream(audio_stream);
	if (err != paNoError) {
		 printf("Error: can't stop stream\n");
	}
	
	err = Pa_CloseStream(audio_stream);
	if (err != paNoError) {
		printf("Error: can't close stream\n");
	}
	Pa_Terminate();
	
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


void initCwd() {
	if (!simulator_cwd_done) {
		getcwd(simulator_cwd, sizeof(simulator_cwd));
	}
	simulator_cwd_done = true;
}



static void display_check_hardware() {
	display_simulator_highlight = !PORTA & _BV(0);
}