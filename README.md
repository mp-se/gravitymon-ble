# This is an example of the new experimenal BLE sender for GravityMon. 

With this alternative option one can send up to 512 bytes of data which is would fit most of the data in the iSpindle dataformat. The format will be customizable in the editor.

The gravitymon BLE sender will support 4 modes:

* Tilt (iBeacon broadcast with Gravity + Temperature)
* Tilt PRO (iBeacon broadcast with Gravity + Temperature, higher accuracy)
* Gravitymon (iBeacon broadcast), works on all boards with bluetooth.
* Gravitymon EddyStoneTLM (BLE 5.x), require an C3 or S3 chip to work.

### The following table shows the client options for receiving the data. 

| Option | Passive Scan | Active Scan | ESP32 | Win/Python |
| :------ | :------: | :------: | :------: | :------: |
| iBeacon/Tilt | Yes | Yes | Yes | Yes |
| iBeacon/Tilt Pro | Yes | Yes | Yes | Yes |
| iBeacon/Gravmon | Yes | Yes | Yes | Yes |
| GravityMon - Ext Advertisement | No | Yes | Yes | No |
| GravityMon - EddyStoneTLM | No | Yes | Yes | Yes |

### The following table shows the data coverage. 

| Option | Gravity | Temp | Angle | Battery | ChipID | 
| :------ | :------: | :------: | :------: | :------: | :------: |     
| iBeacon/Tilt | Yes | Yes | - | - | - | 
| iBeacon/Tilt Pro | Yes | Yes | - | - | - | - | - | - | 
| iBeacon/GravityMon | Yes | Yes | Yes | Yes | Yes | 
| GravityMon - EddyStoneTLM | Yes | Yes | Yes | Yes | Yes | 

## How the GRAVITYMON extended advertisement mode is implemented

This option will send the data as part of an extended advertisement, this allows for up to 252 bytes of data.

Works with the esp32 client in this project but not with a Windows computer nor an iPhone.

# Targets in this project

This project contains the following targets

* server-tilt-c3 : Standard TILT beacon for esp32 c3 board
* server-tilt-ext-c3 : Standard TILT beacon for esp32 c3 board (with EXT advertising enabled)
* server-gravitymon-c3: Gravitymon BLE format for esp32 c3 board
* server-gravitymon-ext-c3: Gravitymon BLE format for esp32 c3 board (with EXT advertising enabled)
* server-adv-c3: Gravitymon BLE extended advertiserment for esp32 c3 board (with EXT advertising enabled)
* client-s3: Client that can connect and read both TILT beacon and Gravitymon advertisement 

Gravitymon BLE ext advertising format requires that the is in ACTIVE mode. Here the payload is part of the advertisement (can be up to 252 chars)

The following defines are used:

**CONFIG_BT_NIMBLE_EXT_ADV=1**  Enabling this will configure the NimBLE library to support extended advertisement. When using this mode its possible to advertise a mix of data options, TILT + Gravitymon for instance.

**The TILT beacon scanner code is based on Thorrak's TILTBRIDGE project.** 
**The TILT beacon is based on the tilt-sim by Spouliot**

# Reading the data

## Testing TILT option

On a linux machine aioblescan can be used to detect tilt devices. Just run:

`aioblescan --tilt`

```
Tilt {"uuid": "a495bb10c5b14b44b5121370f02d74de", "major": 41, "minor": 1234, "tx_power": 0, "rssi": -60, "mac": "c8:c9:a3:cb:38:1a"}
```

If you first test with the tilt option you can find the MAC adress of the BLE chip which is useful to find the right device when testing with the gravitymon option.

## Testing Gravitymon option

`python .\bleak_scan.py --address c8:c9:a3:cb:38:1a`

or

`python .\bleak_scan.py --name gravitymon`

```
2023-10-21 12:32:07,877 __main__ INFO: [Service] 00001800-0000-1000-8000-00805f9b34fb (Handle: 1): Generic Access Profile
2023-10-21 12:32:07,905 __main__ INFO:   [Characteristic] 00002a00-0000-1000-8000-00805f9b34fb (Handle: 2): Device Name (read), Value: bytearray(b'gravitymon')
2023-10-21 12:32:07,934 __main__ INFO:   [Characteristic] 00002a01-0000-1000-8000-00805f9b34fb (Handle: 4): Appearance (read), Value: bytearray(b'\x00\x00')
2023-10-21 12:32:07,935 __main__ INFO: [Service] 00001801-0000-1000-8000-00805f9b34fb (Handle: 6): Generic Attribute Profile
2023-10-21 12:32:07,936 __main__ INFO:   [Characteristic] 00002a05-0000-1000-8000-00805f9b34fb (Handle: 7): Service Changed (indicate)
2023-10-21 12:32:07,966 __main__ INFO:     [Descriptor] 00002902-0000-1000-8000-00805f9b34fb (Handle: 9): Client Characteristic Configuration, Value: bytearray(b'\x02\x00')
2023-10-21 12:32:07,967 __main__ INFO: [Service] ea9a7e4b-d000-483d-8fdb-94b47730ed7a (Handle: 10): Unknown
2023-10-21 12:32:08,012 __main__ INFO:   [Characteristic] ea9a7e4b-d100-483d-8fdb-94b47730ed7a (Handle: 11): Unknown (read), Value: bytearray(b'{"name":"my_device_name","ID": "01234567","token":"my_token","interval":46,"temperature":20.2,"temp_units":"C","gravity":1.05,"angle":
```

## Python test script for reading TILT or GRAVITYMON ble data

`python .\scan.py`
