import asyncio
import logging

from bleak import BleakClient, BleakScanner

logger = logging.getLogger(__name__)

async def main():
    logger.info("starting scan...")

    
    device = await BleakScanner.find_device_by_name("gravitymon", cb=dict(use_bdaddr=False))
    if device is None:
        logger.error("could not find device with name '%s'", "gravitymon")
        return

    logger.info("connecting to device...")

    async with BleakClient(
        device,
    ) as client:
        logger.info("connected")

        for service in client.services:
            for char in service.characteristics:
                if "read" in char.properties and char.uuid.startswith("00002903"):
                    try:
                        value = await client.read_gatt_char(char.uuid)
                        logger.info( value.decode() )
                    except Exception as e:
                        logger.error( "Failed to read, Error: %s", e)
    logger.info("disconnected")

if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)-15s %(name)-8s %(levelname)s: %(message)s",
    )

    asyncio.run(main())