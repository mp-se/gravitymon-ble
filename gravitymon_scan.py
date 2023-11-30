import asyncio, logging, json, requests

from bleak import BleakClient, BleakScanner

logger = logging.getLogger(__name__)

async def main():
    while True:
        logger.info("Scanning for gravitymon devices...")
        
        device = await BleakScanner.find_device_by_name("gravitymon", cb=dict(use_bdaddr=False))
        if device is None:
            logger.error("Could not find any device with name 'gravitymon'")
        else:
            logger.info("Connecting to gravitymon.")
            async with BleakClient(
                device,
            ) as client:
                logger.info("Connected to gravitymon.")
                for service in client.services:
                    for char in service.characteristics:
                        if "read" in char.properties and char.uuid.startswith("00002903"):
                            try:
                                value = await client.read_gatt_char(char.uuid)
                                data = json.loads( value.decode() )
                                logger.info( "Data received: %s", json.dumps(data) )

                            except Exception as e:
                                logger.error( "Failed to read data, Error: %s", e)
            logger.info("Disconnected from gravitymon")

if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)-15s %(name)-8s %(levelname)s: %(message)s",
    )

    asyncio.run(main())
