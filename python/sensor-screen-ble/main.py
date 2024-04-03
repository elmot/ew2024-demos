import time
from pimoroni_i2c import PimoroniI2C
from breakout_sgp30 import BreakoutSGP30
from gfx_pack import GfxPack, SWITCH_A
from ble_co2 import BLETemperature
from ble_advertising import advertising_payload,decode_services,decode_name
import bluetooth
import random


# sets up a handy function we can call to clear the screen
def clear():
    display.set_pen(0)
    display.clear()
    display.set_pen(15)


# From CPython Lib/colorsys.py
def hsv_to_rgb(h, s, v):
    if s == 0.0:
        return v, v, v
    i = int(h * 6.0)
    f = (h * 6.0) - i
    p = v * (1.0 - s)
    q = v * (1.0 - s * f)
    t = v * (1.0 - s * (1.0 - f))
    i = i % 6
    if i == 0:
        return v, t, p
    if i == 1:
        return q, v, p
    if i == 2:
        return p, v, t
    if i == 3:
        return p, q, v
    if i == 4:
        return t, p, v
    if i == 5:
        return v, p, q


i2c = PimoroniI2C(sda=4, scl=5)
sgp30 = BreakoutSGP30(i2c)

print("SGP30 initialised - about to start measuring without waiting")

gp = GfxPack()
display = gp.display

WIDTH, HEIGHT = display.get_bounds()
display.set_backlight(0)  # turn off the white component of the backlight
# the range of readings to map to colours (and scale our graph to)
# https://www.kane.co.uk/knowledge-centre/what-are-safe-levels-of-co-and-co2-in-rooms
MIN = 400
MAX = 2000
# pick what bits of the colour wheel to use (from 0-360°)
# https://www.cssscript.com/demo/hsv-hsl-color-wheel-picker-reinvented/
HUE_START = 100  # green
HUE_END = 0  # red

BRIGHTNESS = 0.5

# the area of the screen we want to draw our graph into
GRAPH_TOP = 24
GRAPH_BOTTOM = 54
highest = 0.0
lowest = 4000.0
readings = []
gp.set_backlight(0, 0, 0, 127)
display.set_font("bitmap8")
display.set_pen(15)
display.text("Waiting for sensor to be ready", 0, 0, WIDTH, 2)
display.update()

sgp30.start_measurement(False)
s_id = sgp30.get_unique_id()
print("Started measuring for id 0x", '{:04x}'.format(s_id[0]), '{:04x}'.format(s_id[1]), '{:04x}'.format(s_id[2]), sep="")

ble = bluetooth.BLE()
temp = BLETemperature(ble)

payload = advertising_payload(
    name="micropython",
    services=[bluetooth.UUID(0x181A), bluetooth.UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")],
)
print(payload)
print(decode_name(payload))
print(decode_services(payload))

t = 25
ti = 0

j = 0
while True:
    if gp.switch_pressed(SWITCH_A):
        # reset recorded high / low values
        highest = 0.0
        lowest = 4000.0

    j += 1
    air_quality = sgp30.get_air_quality()
    eCO2 = air_quality[BreakoutSGP30.ECO2]
    TVOC = air_quality[BreakoutSGP30.TVOC]

    air_quality_raw = sgp30.get_air_quality_raw()
    H2 = air_quality_raw[BreakoutSGP30.H2]
    ETHANOL = air_quality_raw[BreakoutSGP30.ETHANOL]

    print(j, ": CO2 ", eCO2, " TVOC ", TVOC, ", raw ", H2, " ", ETHANOL, sep="")
    if j == 30:
        print("Resetting device")
        sgp30.soft_reset()
        time.sleep(0.5)
        print("Restarting measurement, waiting 15 secs before returning")
        sgp30.start_measurement(True)
        print("Measurement restarted, now read every second")

    # update highest / lowest values
    if eCO2 < lowest:
        lowest = eCO2
    if eCO2 > highest:
        highest = eCO2

    # calculates a colour from the sensor reading
    hue = max(0, HUE_START + ((eCO2 - MIN) * (HUE_END - HUE_START) / (MAX - MIN)))

    # set the leds
    r, g, b = [int(255 * c) for c in hsv_to_rgb(hue / 360, 1.0, BRIGHTNESS)]
    gp.set_backlight(r, g, b, 0)

    # keep track of readings in a list (so we can draw the graph)
    readings.append(eCO2)
    # we only need to save a screen's worth of readings, so delete the oldest
    if len(readings) > WIDTH:
        readings.pop(0)

    # draw the graph
    clear()
    for r in range(len(readings)):
        # this line scales the y axis of the graph into the available space
        y = round(GRAPH_BOTTOM + ((readings[r] - MIN) * (GRAPH_TOP - GRAPH_BOTTOM) / (MAX - MIN)))
        display.pixel(r, y)
    # draw the text
    display.text("CO2", 0, 0, scale=2)
    # measure the rest of the text before drawing so that we can right align it
    text_width = display.measure_text(f"{eCO2:.0f}ppm", scale=2)
    display.text(f"{eCO2:.0f}ppm", WIDTH - text_width, 0, scale=2)

    text_width = display.measure_text(f"TVOC {TVOC:.0f}ppm", scale=1)
    display.text(f"TVOC {TVOC:.0f}ppm", WIDTH - text_width, 16, scale=1)

    display.update()

    # Write BLE every second, notify every 10 seconds.
    ti = (ti + 1) % 10
    temp.set_temperature(t, notify=ti == 0, indicate=False)
    # Random walk the temperature.
    t += random.uniform(-0.5, 0.5)

    time.sleep(1.0)



