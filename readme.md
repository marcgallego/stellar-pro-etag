# Stellar Pro ESL custom firmware

## Supported models

- Hanshow Stellar Pro 290R-N (supported, tested, and used as the main development device)
- Hanshow Stellar Pro 213R-N (not tested but should be supported)

## How to flash?

1. Remove the battery cover and check if the mainboard looks like the diagram below.

![Wiring diagram](/images/flashing_connection_usb_to_uart.jpg)

1. Solder four wires: GND, VCC, RX, RTS.
2. Use a USB-to-TTL module (CH340) to connect the four wires. Connect RX to TX, TX to RX, VCC to 3.3V, GND to GND. Wire the RTS pin to pin 3 of the CH340G chip (or skip soldering and briefly connect RTS to GND before flashing).
3. Open <https://atc1441.github.io/3TC_TLSR_Paper_UART_Flasher.html>. Keep the default baud rate of 460800, leave Atime as default, and select the file Firmware/ATC_Paper.bin.
4. Click "Unlock" first, then click "Write to Flash" and wait for completion. Once successful, the screen will refresh automatically.

## Building the Project

```cmd
cd Firmware
makeit.exe clean && makeit.exe -j12
```

Output on success:

```
'Create Flash image (binary format)'
'Invoking: TC32 Create Extended Listing'
'Invoking: Print Size'
"tc32_windows\\bin\\"tc32-elf-size -t ./out/ATC_Paper.elf
copy from `./out/ATC_Paper.elf' [elf32-littletc32] to `./out/../ATC_Paper.bin' [binary]
   text    data     bss     dec     hex filename
  75608    4604   25341  105553   19c51 ./out/ATC_Paper.elf
  75608    4604   25341  105553   19c51 (TOTALS)
'Finished building: sizedummy'
' '
tl_fireware_tools.py v0.1 dev
Firmware CRC32: 0xe62d501e
'Finished building: out/../ATC_Paper.bin'
' '
'Finished building: out/ATC_Paper.lst'
' '
```

## Bluetooth Connection and OTA Update

1. You must disconnect the TTL TX wire first, otherwise Bluetooth connection will fail.
2. OTA update: <https://atc1441.github.io/ATC_TLSR_Paper_OTA_writing.html>

### Uploading Images

1. Run `cd web_tools && python -m http.server`
2. Open <http://127.0.0.1:8000> and connect via Bluetooth on the page.
3. Select and upload an image. After uploading, you can add text or draw manually. You can also set the dithering algorithm.
4. Send to the device and wait for the screen to refresh.

### Apple FindMy Network / AirTag Emulation

- The device supports joining the Apple FindMy network. (The device broadcasts AirTag-compatible public keys via Bluetooth. When nearby Apple devices receive the public key, they encrypt their location data with it and send it to the FindMy server. You can then use your private key to retrieve the device's location from Apple's servers.)
- This feature is disabled by default.
- To enable it, modify the `PUB_KEY=` value in `ble.c` to your own public key. For instructions on generating keys, see: (<https://github.com/dchristl/macless-haystack> or <https://github.com/malmeloo/openhaystack>)
- You also need to set `AIR_TAG_OPEN=1` in `ble.c`.

### Features

- [X] Python image generation script
- [X] Bluetooth image upload with incorrect display size
- [X] Added BLE notification after image upload
- [X] Added scenes with mode switching support
- [X] Image mode
- [X] Web image switching support
- [X] Added new clock scene
- [X] Support for setting year/month/day
- [X] Web drawing editor, direct image upload, B&W dithering
- [X] Tri-color dithering, device-side tri-color display, BLE transfer support
- [X]  EPD buffer refresh data anomaly (occasional black bars on left/right)
- [X]  Apple FindMy network support / AirTag emulation

### Acknowledgments

I would like to thank the following projects and its contributors upon which this project is based:

- [ATC_TLSR_Paper](https://github.com/atc1441/ATC_TLSR_Paper), by atc1441.
- [stellar-L3N-etag](https://github.com/reece15/stellar-L3N-etag), by reece15.

### References

- [TLSR8359 Datasheet](docs/Datasheet_Telink_ULP_2.4GHz_RF_SoC_TLSR8359.pdf)
- [TLSR8x5x BLE SDK Developer Handbook](docs/Telink_Kite_BLE_SDK_Developer_Handbook.pdf)
- [Display Driver Datasheet - SSD1680.pdf](/docs/SSD1680.pdf)
