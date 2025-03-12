import asyncio
import logging
import json
import requests
import time
import os
from uuid import UUID

from construct import Array, Byte, Const, Int8sl, Int16ub, Int32ub, Struct
from construct.core import ConstError

from bleak import BleakScanner, BleakClient
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

logger = logging.getLogger("tilt")
skip_push = True
endpoint_gravity = "http://" + os.getenv("API_HOST") + "/api/gravity/public"
endpoint_pressure = "http://" + os.getenv("API_HOST") + "/api/gravity/public"
headers = {
    "Content-Type": "application/json",
}

minium_interval = 0

gravitymon_tilt_format = Struct(
    "type_length" / Const(b"\x02\x15"),
    "uuid" / Array(16, Byte),
    "major" / Int16ub,
    "minor" / Int16ub,
    "power" / Int8sl,
)

gravitymon_ibeacon_format = Struct(
    "type_length" / Const(b"\x03\x15"),
    "name" / Const(b"GRAVMON."),
    # "name" / Array(8, Byte),
    "chipid" / Int32ub,
    "angle" / Int16ub,
    "battery" / Int16ub,
    "gravity" / Int16ub,
    "temp" / Int16ub,
)

gravitymon_eddystone_format = Struct(
    "type_length" / Const(b"\x20\x00"),
    "battery" / Int16ub,
    "temp" / Int16ub,
    "gravity" / Int16ub,
    "angle" / Int16ub,
    "chipid" / Int32ub,
)

pressuremon_ibeacon_format = Struct(
    "type_length" / Const(b"\x03\x15"),
    "name" / Const(b"PRESMON."),
    # "name" / Array(8, Byte),
    "chipid" / Int32ub,
    "pressure" / Int16ub,
    "pressure1" / Int16ub,
    "battery" / Int16ub,
    "temp" / Int16ub,
)

# pressuremon_eddystone_format = Struct(
#     "type_length" / Const(b"\x20\x00"),
#     "battery" / Int16ub,
#     "temp" / Int16ub,
#     "pressure" / Int16ub,
#     "pressure1" / Int16ub,
#     "chipid" / Int32ub,
# )

chamber_ibeacon_format = Struct(
    "type_length" / Const(b"\x03\x15"),
    "name" / Const(b"CHAMBER."),
    # "name" / Array(8, Byte),
    "chipid" / Int32ub,
    "chamberTemp" / Int16ub,
    "beerTemp" / Int16ub,
)

class tilt:
    def __init__(self, color, uuid, time):
        self.color = color
        self.uuid = uuid
        self.time = time


# List of tilts (class tilt)
tilts = []
# Dict of gravitymon devices (ID: time)
gravitymons = {}
# Dict of pressuremon devices (ID: time)
pressuremons = {}


def init():
    tilts.append(
        tilt(
            "red",
            UUID("A495BB10-C5B1-4B44-B512-1370F02D74DE"),
            time.time() - minium_interval,
        )
    )
    tilts.append(
        tilt(
            "green",
            UUID("A495BB20-C5B1-4B44-B512-1370F02D74DE"),
            time.time() - minium_interval,
        )
    )
    tilts.append(
        tilt(
            "black",
            UUID("A495BB30-C5B1-4B44-B512-1370F02D74DE"),
            time.time() - minium_interval,
        )
    )
    tilts.append(
        tilt(
            "purple",
            UUID("A495BB40-C5B1-4B44-B512-1370F02D74DE"),
            time.time() - minium_interval,
        )
    )
    tilts.append(
        tilt(
            "orange",
            UUID("A495BB50-C5B1-4B44-B512-1370F02D74DE"),
            time.time() - minium_interval,
        )
    )
    tilts.append(
        tilt(
            "blue",
            UUID("A495BB60-C5B1-4B44-B512-1370F02D74DE"),
            time.time() - minium_interval,
        )
    )
    tilts.append(
        tilt(
            "yellow",
            UUID("A495BB70-C5B1-4B44-B512-1370F02D74DE"),
            time.time() - minium_interval,
        )
    )
    tilts.append(
        tilt(
            "pink",
            UUID("A495BB80-C5B1-4B44-B512-1370F02D74DE"),
            time.time() - minium_interval,
        )
    )


def contains(list, filter):
    for x in list:
        if filter(x):
            return True
    return False


def first(iterable, default=None):
    for item in iterable:
        return item
    return default


async def parse_gravitymon(device: BLEDevice, advertisement_data: AdvertisementData):
    global gravitymons

    try:
        apple_data = advertisement_data.manufacturer_data[0x004C]
        ibeacon = gravitymon_ibeacon_format.parse(apple_data)

        logger.info(f"Parsing gravitymon ibeacon: {device}")

        data = {
            "name": "",
            "ID": hex(ibeacon.chipid)[2:],
            "token": "",
            "interval": 0,
            "battery": ibeacon.battery / 1000,
            "gravity": ibeacon.gravity / 10000,
            "angle": ibeacon.angle / 100,
            "temperature": ibeacon.temp / 1000,
            "temp_units": "C",
            "RSSI": 0,
        }
        logger.info(f"Gravitymon data received: {json.dumps(data)} {device.address}")

        now = time.time()
        logger.debug(
            f"Found gravitymon device, checking if time has expired, min={minium_interval}s"
        )

        if (
            abs(gravitymons.get(data["ID"], now - minium_interval * 2) - now)
            > minium_interval
        ):
            gravitymons[data["ID"]] = now
            logger.info(f"Gravitymon data received: {json.dumps(data)}")
            if not skip_push:
                try:
                    logger.info("Posting gravitymon data.")
                    r = requests.post(endpoint_gravity, json=data, headers=headers)
                    logger.info(f"Response {r}.")
                except Exception as e:
                    logger.error(f"Failed to post gravitymon data, Error: {e}")
    except KeyError:
        pass
    except ConstError:
        pass
    

def parse_gravitymon_eddystone(device: BLEDevice, advertisement_data: AdvertisementData):
    global gravitymons

    try:
        uuid = advertisement_data.service_uuids[0]
        data = advertisement_data.service_data.get(uuid)
        eddy = gravitymon_eddystone_format.parse(data)

        logger.info(f"Parsing gravitymon eddystone: {device}")

        data = {
            "name": "",
            "ID": hex(eddy.chipid)[2:],
            "token": "",
            "interval": 0,
            "battery": eddy.battery / 1000,
            "gravity": eddy.gravity / 10000,
            "angle": eddy.angle / 100,
            "temperature": eddy.temp / 1000,
            "temp_units": "C",
            "RSSI": 0,
        }
        logger.info(f"Gravitymon data received: {json.dumps(data)} {device.address}")

        now = time.time()
        logger.debug(
            f"Found gravitymon device, checking if time has expired, min={minium_interval}s"
        )

        if (
            abs(gravitymons.get(data["ID"], now - minium_interval * 2) - now)
            > minium_interval
        ):
            gravitymons[data["ID"]] = now
            logger.info(f"Gravitymon data received: {json.dumps(data)}")
            if not skip_push:
                try:
                    logger.info("Posting gravitymon data.")
                    r = requests.post(endpoint_gravity, json=data, headers=headers)
                    logger.info(f"Response {r}.")
                except Exception as e:
                    logger.error(f"Failed to post gravitymon data, Error: {e}")
    except KeyError:
        pass
    except ConstError:
        pass

async def parse_pressuremon(device: BLEDevice, advertisement_data: AdvertisementData):
    global gravitymons

    try:
        apple_data = advertisement_data.manufacturer_data[0x004C]
        ibeacon = pressuremon_ibeacon_format.parse(apple_data)

        logger.info(f"Parsing pressuremon ibeacon: {device}")

        data = {
            "name": "",
            "ID": hex(ibeacon.chipid)[2:],
            "token": "",
            "interval": 0,
            "battery": ibeacon.battery / 1000,
            "pressure": ibeacon.pressure / 100,
            "pressure1": ibeacon.pressure1 / 100,
            "temperature": ibeacon.temp / 1000,
            "pressure-unit": "PSI",
            "temperature-unit": "C",
            "RSSI": 0,
        }
        logger.info(f"Pressuremon data received: {json.dumps(data)} {device.address}")

        now = time.time()
        logger.debug(
            f"Found pressuremon device, checking if time has expired, min={minium_interval}s"
        )

        if (
            abs(pressuremons.get(data["ID"], now - minium_interval * 2) - now)
            > minium_interval
        ):
            pressuremons[data["ID"]] = now
            logger.info(f"Pressuremon data received: {json.dumps(data)}")
            if not skip_push:
                try:
                    logger.info("Posting pressuremon data.")
                    r = requests.post(endpoint_pressure, json=data, headers=headers)
                    logger.info(f"Response {r}.")
                except Exception as e:
                    logger.error(f"Failed to post pressuremon data, Error: {e}")
    except KeyError:
        pass
    except ConstError:
        pass

async def parse_chamber(device: BLEDevice, advertisement_data: AdvertisementData):
    try:
        apple_data = advertisement_data.manufacturer_data[0x004C]
        ibeacon = chamber_ibeacon_format.parse(apple_data)

        logger.info(f"Parsing chamber ibeacon: {device}")

        data = {
            "ID": hex(ibeacon.chipid)[2:],
            "chamber-temp": ibeacon.chamberTemp / 1000,
            "beer-temp": ibeacon.beerTemp / 1000,
            "temperature-unit": "C",
        }
        logger.info(f"Chamber data received: {json.dumps(data)} {device.address}")
    except KeyError:
        pass
    except ConstError:
        pass

# def parse_pressuremon_eddystone(device: BLEDevice, advertisement_data: AdvertisementData):
#     global pressuremons

#     try:
#         uuid = advertisement_data.service_uuids[0]
#         data = advertisement_data.service_data.get(uuid)
#         eddy = pressuremon_eddystone_format.parse(data)

#         logger.info(f"Parsing pressuremon eddystone: {device}")

#         data = {
#             "name": "",
#             "ID": hex(eddy.chipid)[2:],
#             "token": "",
#             "interval": 0,
#             "battery": eddy.battery / 1000,
#             "pressure": eddy.pressure / 100,
#             "pressure1": eddy.pressure1 / 100,
#             "temperature": eddy.temp / 1000,
#             "pressure-unit": "PSI",
#             "temperature-unit": "C",
#             "RSSI": 0,
#         }
#         logger.info(f"Pressuremmon data received: {json.dumps(data)} {device.address}")

#         now = time.time()
#         logger.debug(
#             f"Found pressuremon device, checking if time has expired, min={minium_interval}s"
#         )

#         if (
#             abs(pressuremons.get(data["ID"], now - minium_interval * 2) - now)
#             > minium_interval
#         ):
#             pressuremons[data["ID"]] = now
#             logger.info(f"Pressuremon data received: {json.dumps(data)}")
#             if not skip_push:
#                 try:
#                     logger.info("Posting pressuremon data.")
#                     r = requests.post(endpoint_pressure, json=data, headers=headers)
#                     logger.info(f"Response {r}.")
#                 except Exception as e:
#                     logger.error(f"Failed to post pressuremon data, Error: {e}")
#     except KeyError:
#         pass
#     except ConstError:
#         pass


def parse_gravitymon_tilt(advertisement_data: AdvertisementData):
    try:
        apple_data = advertisement_data.manufacturer_data[0x004C]
        ibeacon = gravitymon_tilt_format.parse(apple_data)
        uuid = UUID(bytes=bytes(ibeacon.uuid))
        tilt = first(x for x in tilts if x.uuid == uuid)

        if tilt is not None:
            logger.debug(
                f"Found tilt device, checking if time has expired, min={minium_interval}s"
            )
            now = time.time()

            if abs(tilt.time - now) > minium_interval:
                tilt.time = now

                if ibeacon.minor > 5000: # Check if the data is related to TILT PRO (higher resolution)
                    tempF = ibeacon.major / 10
                    gravitySG = ibeacon.minor / 10000
                else:
                    tempF = ibeacon.major
                    gravitySG = ibeacon.minor / 1000

                data = {
                    "color": tilt.color,
                    "gravity": gravitySG,
                    "temperature": tempF,
                    "RSSI": advertisement_data.rssi,
                }

                logger.info(f"Tilt data received: {json.dumps(data)}")
                if not skip_push:
                    try:
                        logger.info("Posting tilt data.")
                        r = requests.post(endpoint_gravity, json=data, headers=headers)
                        logger.info(f"Response {r}.")
                    except Exception as e:
                        logger.error(f"Failed to post tilt data, Error: {e}")
    except KeyError:
        pass
    except ConstError:
        pass


async def device_found(device: BLEDevice, advertisement_data: AdvertisementData):
    # logger.info(f"Found: {device.name} {advertisement_data.service_uuids}")

    if device.name == "gravitymon" and any(
        "0000feaa-" in s for s in advertisement_data.service_uuids
    ):
        parse_gravitymon_eddystone(device=device, advertisement_data=advertisement_data)
    # elif device.name == "pressuremon" and any(
    #     "0000feaa-" in s for s in advertisement_data.service_uuids
    # ):
    #     parse_pressuremon_eddystone(device=device, advertisement_data=advertisement_data)
    else:
        # Try the other formats and see what matches
        await parse_gravitymon(device=device, advertisement_data=advertisement_data)
        await parse_pressuremon(device=device, advertisement_data=advertisement_data)
        await parse_chamber(device=device, advertisement_data=advertisement_data)
        parse_gravitymon_tilt(advertisement_data=advertisement_data)


async def main():
    global minium_interval

    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)-15s %(name)-8s %(levelname)s: %(message)s",
    )

    t = os.getenv("MIN_INTERVAL")
    minium_interval = 5 * 60  # seconds
    if t is not None:
        minium_interval = int(t)
    logger.info(f"Minium interval = {minium_interval}, reporting to {endpoint_gravity} + {endpoint_pressure}")

    init()
    scanner = BleakScanner(detection_callback=device_found, scanning_mode="active")

    logger.info("Scanning for tilt/gravitymon/pressuremon BLE devices...")
    while True:
        await scanner.start()
        await asyncio.sleep(0.1)
        await scanner.stop()


asyncio.run(main())
logger.info("Exit from scanner")
