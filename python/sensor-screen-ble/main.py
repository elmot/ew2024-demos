import time
from pimoroni_i2c import PimoroniI2C
from breakout_sgp30 import BreakoutSGP30
from gfx_pack import GfxPack, SWITCH_A

from micropython import const

import uasyncio as asyncio
import aioble
import bluetooth

import struct
import machine

# ############ Temperature
sensor = machine.ADC(4)


def read_temperature():
    adc_value = sensor.read_u16()
    volt = (3.3 / 65535) * adc_value
    temperature = 27 - (volt - 0.706) / 0.001721
    return round(temperature, 1)


# ############ BLE
# Helper to encode the temperature characteristic encoding (sint16, hundredths of a degree).
def _encode_temperature(temp_deg_c):
    return struct.pack("<h", int(temp_deg_c * 100))


def _encode_co2(e_co2):
    return struct.pack("<i", int(e_co2))


# org.bluetooth.service.environmental_sensing
_ENV_SENSE_UUID = bluetooth.UUID(0x181A)
# org.bluetooth.characteristic.temperature
_ENV_SENSE_TEMP_UUID = bluetooth.UUID(0x2A6E)
_ENV_SENSE_CO2_UUID = bluetooth.UUID(0x2B8C)
# org.bluetooth.characteristic.gap.appearance.xml
_ADV_APPEARANCE_GENERIC_THERMOMETER = const(768)
_ADV_APPEARANCE_GENERIC_ENV_SENSOR = const(5696)

# How frequently to send advertising beacons.
_ADV_INTERVAL_MS = 250_000


# This would be periodically polling a hardware sensor.
async def sensor_task():
    j = 0
    highest = 0.0
    lowest = 4000.0
    readings = []
    while True:
        if gp.switch_pressed(SWITCH_A):
            # reset recorded high / low values
            highest = 0.0
            lowest = 4000.0

        j += 1
        air_quality = sgp30.get_air_quality()
        e_co2 = air_quality[BreakoutSGP30.ECO2]
        tvoc = air_quality[BreakoutSGP30.TVOC]

        air_quality_raw = sgp30.get_air_quality_raw()
        h2 = air_quality_raw[BreakoutSGP30.H2]
        ethanol = air_quality_raw[BreakoutSGP30.ETHANOL]

        print(j, ": CO2 ", e_co2, " TVOC ", tvoc, ", raw ", h2, " ", ethanol, sep="")
        if j == 30:
            print("Resetting device")
            sgp30.soft_reset()
            time.sleep(0.5)
            print("Restarting measurement, waiting 15 secs before returning")
            sgp30.start_measurement(False)
            await asyncio.sleep_ms(15000)
            print("Measurement restarted, now read every second")

        # update highest / lowest values
        if e_co2 < lowest:
            lowest = e_co2
        if e_co2 > highest:
            highest = e_co2

        # calculates a colour from the sensor reading
        hue = max(0, HUE_START + ((e_co2 - MIN) * (HUE_END - HUE_START) // (MAX - MIN)))

        # set the leds
        r, g, b = [int(255 * c) for c in hsv_to_rgb(hue / 360, 1.0, BRIGHTNESS)]
        gp.set_backlight(r, g, b, 0)

        # keep track of readings in a list (so we can draw the graph)
        readings.append(e_co2)
        # we only need to save a screen's worth of readings, so delete the oldest
        if len(readings) > WIDTH:
            readings.pop(0)

        t = read_temperature()
        # draw the graph
        clear()
        for r in range(len(readings)):
            # this line scales the y axis of the graph into the available space
            y = round(GRAPH_BOTTOM + ((readings[r] - MIN) * (GRAPH_TOP - GRAPH_BOTTOM) / (MAX - MIN)))
            display.pixel(r, y)
        # draw the text
        display.text("CO2", 0, 0, scale=2)
        # measure the rest of the text before drawing so that we can right align it
        text_width = display.measure_text(f"{e_co2:.0f}ppm", scale=2)
        display.text(f"{e_co2:.0f}ppm", WIDTH - text_width, 0, scale=2)

        text_width = display.measure_text(f"TVOC {tvoc:.0f}ppm", scale=1)
        display.text(f"TVOC {tvoc:.0f}ppm", WIDTH - text_width, 16, scale=1)

        text_width = display.measure_text(f"Temp {t:.1f} C", scale=1)
        display.text(f"Temp {t:.1f} C", WIDTH // 2 - text_width, 16, scale=1)

        display.update()
        temp_characteristic.write(_encode_temperature(t), send_update=True)
        co2_characteristic.write(_encode_co2(e_co2), send_update=True)

        await asyncio.sleep_ms(1000)


# Serially wait for connections. Don't advertise while a central is
# connected.
async def peripheral_task():
    while True:
        async with await aioble.advertise(
                _ADV_INTERVAL_MS,
                name="mpy-temp",
                services=[_ENV_SENSE_UUID],
                appearance=_ADV_APPEARANCE_GENERIC_THERMOMETER,
        ) as connection:
            print("Connection from", connection.device)
            await connection.disconnected(timeout_ms=-1)


# Run both tasks.
async def main():
    t1 = asyncio.create_task(sensor_task())
    t2 = asyncio.create_task(peripheral_task())
    # noinspection PyCallingNonCallable
    await asyncio.gather(t1, t2, return_exceptions=True)


# ############ DISPLAY
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
gp.set_backlight(0, 0, 0, 127)
display.set_font("bitmap8")
display.set_pen(15)
display.text("Waiting for sensor to be ready", 0, 0, WIDTH, 2)
display.update()

sgp30.start_measurement(False)
s_id = sgp30.get_unique_id()
print("Started measuring for id 0x", '{:04x}'.format(s_id[0]), '{:04x}'.format(s_id[1]), '{:04x}'.format(s_id[2]),
      sep="")

# Register GATT server.
env_service = aioble.Service(_ENV_SENSE_UUID)
temp_characteristic = aioble.Characteristic(
    env_service, _ENV_SENSE_TEMP_UUID, read=True, notify=True
)
co2_characteristic = aioble.Characteristic(
    env_service, _ENV_SENSE_CO2_UUID, read=True, notify=True
)
aioble.register_services(env_service)

asyncio.run(main())
