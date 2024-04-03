#include <cstring>

#include "libraries/pico_display_2/pico_display_2.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "rgbled.hpp"

using namespace pimoroni;

// PicoDisplay2 is 320 by 240
#define DISPLAY_WIDTH PicoDisplay2::WIDTH
#define DISPLAY_HEIGHT PicoDisplay2::HEIGHT

ST7789 st7789(DISPLAY_WIDTH, DISPLAY_HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
PicoGraphics_PenRGB565 graphics(st7789.width, st7789.height, nullptr);

RGBLED led(PicoDisplay2::LED_R, PicoDisplay2::LED_G, PicoDisplay2::LED_B);

static Pen BG = graphics.create_pen(120, 40, 60);
static Pen WHITE = graphics.create_pen(255, 255, 255);
static Pen TEXT = graphics.create_pen(50, 255, 155);

void setupDisplay() {
    st7789.set_backlight(255);
    Point text_location(0, 0);


    graphics.set_pen(BG);
    graphics.clear();
    graphics.set_pen(WHITE);
    graphics.text("Waiting for BT", text_location, 320, 4);

    // update screen
    st7789.update(&graphics);

}

// the range of readings to map to colours (and scale our graph to)
// https://www.kane.co.uk/knowledge-centre/what-are-safe-levels-of-co-and-co2-in-rooms
constexpr int MIN = 400;
constexpr int MAX = 2000;
// pick what bits of the colour wheel to use (from 0-360°)
// https://www.cssscript.com/demo/hsv-hsl-color-wheel-picker-reinvented/
constexpr float HUE_START = 100;// green
constexpr float HUE_END = 0;    // red
constexpr float BRIGHTNESS = 0.5;

static struct {
    int32_t data[DISPLAY_WIDTH];
    size_t start_ptr;
    size_t end_ptr;
} readings = {.data{}, .start_ptr = 0, .end_ptr=0};
int max_ppm = -10000;
int min_ppm = 1000001;

void displayCO2(int32_t co2_ppm) {
    static char buffer[40];
    graphics.set_pen(BG);
    graphics.clear();

    sniprintf(buffer, sizeof buffer, "CO2: %lippm", co2_ppm);
    graphics.set_font(&hershey::futuram);
    graphics.set_pen(TEXT);
    graphics.text(buffer, Point(0, 25), DISPLAY_WIDTH, 1.2, 0.3);
    float hue = MAX(0, HUE_START + ((co2_ppm - MIN) * (HUE_END - HUE_START) / (MAX - MIN)));
    led.set_hsv(hue / 360, 1.0, BRIGHTNESS);
    readings.data[readings.end_ptr] = co2_ppm;
    max_ppm = MAX(max_ppm, co2_ppm);
    min_ppm = MIN(min_ppm, co2_ppm);
    readings.end_ptr = (readings.end_ptr + 1) % DISPLAY_WIDTH;
    if (readings.end_ptr == readings.start_ptr) {
        readings.start_ptr = (readings.end_ptr + 1) % DISPLAY_WIDTH;
    }
    graphics.set_pen(WHITE);
    Point b;
    for (int i = (int) readings.start_ptr, x = 0; i != readings.end_ptr; i = (i + 1) % DISPLAY_WIDTH, ++x) {
        auto v = (float) readings.data[(i + readings.start_ptr) % DISPLAY_WIDTH];
        auto delta = max_ppm - min_ppm;
        if (delta == 0) delta = 10;
        auto y = 42 + (DISPLAY_HEIGHT - 45) * ((float) max_ppm - v) / (float) delta;
        auto a = Point(x, (int) y);
        if (i > 0) {
            graphics.line(a, b);
        }
        b = a;
    }
    st7789.update(&graphics);
}
