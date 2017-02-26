
name = 'zx-tape-loader'

simulation = False

src = [
   'src/lib/glcd/*.c', 'src/lib/glcd/controllers/*.c', 'src/lib/glcd/devices/*.c', 'src/lib/sdreader/*.c',
   'src/*.c', 'src/formats/*.c',   
   'src/asm/*.s'
]

mcu = 'atmega128'

frequency = 16*1000000

port = ''

baudrate = 1200

programmer = 'arduino'

defines = [
   'GLCD_DEVICE_AVR8', 'GLCD_CONTROLLER_PCD8544', 'GLCD_USE_AVR_DELAY', '__DELAY_BACKWARD_COMPATIBLE__', 'F_CPU=16000000', 
   'DEBUG=0'
]

compiler_options = ['-g2']

linker_options = []


configurations = {
	'8563-v1_0': {
		'name': 'tape-simulator_8563_hwv-1_0',
		'define': ['OPTIONS_FORM_MAKE_FILE=1', 'USE_RTC_8563=1', 'USE_RTC_8583=0', 'DEBUG=0', 'NDEBUG']
	},
	'8583-v1_0': {
		'name': 'tape-simulator_8583_hwv-1_0',
		'define': ['OPTIONS_FORM_MAKE_FILE=1', 'USE_RTC_8583=1', 'USE_RTC_8563=0', 'DEBUG=0', 'NDEBUG']
	},
	'8563-invert-lcd-v1_2': {
		'name': 'tape-simulator_8563_hwv-1_2-invert_lcd',
		'define': ['OPTIONS_FORM_MAKE_FILE=1', 'USE_RTC_8563=1', 'USE_RTC_8583=0', 'DEBUG=0', 'NDEBUG', 'DISPLAY_HIGHLIGHT_INVERT=1']
	},
	'8583-invert-lcd-v1_2': {
		'name': 'tape-simulator_8583_hwv-1_2-invert_lcd',
		'define': ['OPTIONS_FORM_MAKE_FILE=1', 'USE_RTC_8583=1', 'USE_RTC_8563=0', 'DEBUG=0', 'NDEBUG', 'DISPLAY_HIGHLIGHT_INVERT=1']
	},
	'8563-v1_2': {
		'name': 'tape-simulator_8563_hwv-1_2',
		'define': ['OPTIONS_FORM_MAKE_FILE=1', 'USE_RTC_8563=1', 'USE_RTC_8583=0', 'DEBUG=0', 'NDEBUG', 'DISPLAY_HIGHLIGHT_INVERT=0']
	},
	'8583-v1_2': {
		'name': 'tape-simulator_8583_hwv-1_2',
		'define': ['OPTIONS_FORM_MAKE_FILE=1', 'USE_RTC_8583=1', 'USE_RTC_8563=0', 'DEBUG=0', 'NDEBUG', 'DISPLAY_HIGHLIGHT_INVERT=0']
	}   
   
}

if simulation:
	compiler = 'gcc'
	libs = 'sdl2 portaudio-2.0'
	include = ['src/simulator/stubs']
	src.append('src/simulator/*.c');
	src.append('src/simulator/asm/*.c');
	src.append('src/simulator/stubs/*.c');
	src.remove('src/lib/sdreader/*.c')
	src.remove('src/lib/glcd/devices/*.c')
	src.remove('src/lib/glcd/controllers/*.c')
	defines.append('SIMULATION')
	defines.append('bool=uint8_t')
	defines.append('true=1')
	defines.append('false=0')