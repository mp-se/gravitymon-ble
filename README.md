# This is an example of the new experimenal BLE sender for GravityMon. 

With this alternative option one can send up to 512 bytes of data which is would fit most of the data in the iSpindle dataformat. The format will be customizable in the editor.

The gravitymon BLE sender will support 3 modes:

* Tilt 
* Tilt PRO
* Gravitymon 

# How it will be implemented

So far this is work in progress and the intended way it would work is as follows.

1) Gravitymon will send out the BLE advertisement for up to x seconds (timeout will be configurable)
2) A client will connect to the device and read the characteristic which will be a json string (format will be configurable), we are limited to 512 bytes of data.
3) Once the reading has been done or the timeout expires the gravitymon deviec will go into sleep mode.

Its expected to double the battery life for the device compared to sending the same data over wifi. More tests on coverage and distance is needed before this is implemented. 

# Testing TILT option

On a linux machine aioblescan can be used to detect tilt devices. Just run:

`aioblescan --tilt`

```
Tilt {"uuid": "a495bb10c5b14b44b5121370f02d74de", "major": 41, "minor": 1234, "tx_power": 0, "rssi": -60, "mac": "c8:c9:a3:cb:38:1a"}
```

If you first test with the tilt option you can find the MAC adress of the BLE chip which is useful to find the right device when testing with the gravitymon option.

# Testing Gravitymon option

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

