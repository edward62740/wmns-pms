// Setup for the ESP32 S2 with ST7789 display
// Note SPI DMA with ESP32 S2 is not currently supported

// See SetupX_Template.h for all options available

#define ST7789_DRIVER     // Configure all registers



#define TFT_DC 10
#define TFT_MOSI 7
#define TFT_SCLK 6
#define TFT_RST 5
#define TFT_MISO 8
#define TFT_CS 2

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define SPI_FREQUENCY   27000000

#define SPI_TOUCH_FREQUENCY  2500000
