import asyncio, logging, json, requests, time, os
from uuid import UUID

from construct import Array, Byte, Const, Int8sl, Int16ub, Struct
from construct.core import ConstError

from bleak import BleakScanner, BleakClient
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

logger = logging.getLogger(__name__)

async def device_found(
    device: BLEDevice, advertisement_data: AdvertisementData
):
    try:
        if device.name != "gravitymon":
            return
  
        logger.info("Connecting to gravitymon.")
        async with BleakClient(
            device,
        ) as client:
            logger.info("Connected to gravitymon.")
            for service in client.services:
                if service.uuid.startswith("0000180a-"):
                    for char in service.characteristics:
                        if "read" in char.properties and char.uuid.startswith("00002900-"):
                            try:
                                value = await client.read_gatt_char(char.uuid)
                                data = json.loads( value.decode() )
                                logger.info( "Data received: %s", json.dumps(data) )

                            except Exception as e:
                                logger.error( "Failed to read data, Error: %s", e)
            
            await client.disconnect()           
            logger.info("Disconnected from gravitymon")

    except Exception as e:
        logger.error("Exception %s", e)

async def main():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)-15s %(name)-8s %(levelname)s: %(message)s",
    )

    scanner = BleakScanner(detection_callback=device_found,scanning_mode="passive")
    #scanner = BleakScanner(detection_callback=device_found,scanning_mode="active")

    logger.info("Scanning for gravitymon devices...")

    while True:
        await scanner.start()
        await asyncio.sleep(0.1)
        await scanner.stop()

asyncio.run(main())