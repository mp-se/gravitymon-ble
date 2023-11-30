import asyncio, logging, json, requests, time, os
from uuid import UUID

from construct import Array, Byte, Const, Int8sl, Int16ub, Struct
from construct.core import ConstError

from bleak import BleakScanner
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

logger = logging.getLogger(__name__)

ibeacon_format = Struct(
    "type_length" / Const(b"\x02\x15"),
    "uuid" / Array(16, Byte),
    "major" / Int16ub,
    "minor" / Int16ub,
    "power" / Int8sl,
)

class tilt:
    def __init__(self, color, uuid):
        self.color = color
        self.uuid = uuid

tilts = []

def init():
    tilts.append(tilt("red", UUID("A495BB10-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("green", UUID("A495BB20-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("black", UUID("A495BB30-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("purple", UUID("A495BB40-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("orange", UUID("A495BB50-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("blue", UUID("A495BB60-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("yellow", UUID("A495BB70-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("pink", UUID("A495BB80-C5B1-4B44-B512-1370F02D74DE")))

def contains(list, filter):
    for x in list:
        if filter(x):
            return True
    return False

def first(iterable, default=None):
  for item in iterable:
    return item
  return default

def device_found(
    device: BLEDevice, advertisement_data: AdvertisementData
):
    try:
        apple_data = advertisement_data.manufacturer_data[0x004C]
        ibeacon = ibeacon_format.parse(apple_data)
        uuid = UUID(bytes=bytes(ibeacon.uuid))
        tilt = first(x for x in tilts if x.uuid == uuid)

        if tilt is not None:
            tempF = ibeacon.major
            gravitySG = ibeacon.minor/1000

            data = {
                "color": tilt.color,
                "gravity": gravitySG,
                "temperature": tempF,
                "RSSI": advertisement_data.rssi,
            }

            logger.info( "Data received: %s", json.dumps(data) )
            #logger.info( "Device adress: %s", device.address )

    except KeyError as e:
        pass
    except ConstError as e:
        pass

async def main():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)-15s %(name)-8s %(levelname)s: %(message)s",
    )

    init()
    scanner = BleakScanner(detection_callback=device_found,scanning_mode="active")

    logger.info("Scanning for tilt devices...")

    while True:
        await scanner.start()
        await asyncio.sleep(0.1)
        await scanner.stop()

asyncio.run(main())