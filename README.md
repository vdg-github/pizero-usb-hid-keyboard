# pizero-usb-hid-keyboard

## Pi Zero W (USB Gadget Mode)

### Procedure

Basically, a ripoff of this: https://www.aidanwoods.com/blog/building-a-wifi-enabled-usb-rubber-ducky/

Tested on Raspberry Pi Zero W and a Windows 10 PC (version 1809).

1. Started with a clean install of Raspbian Buster
2. Used a Raspberry Pi 3 to setup wifi/keyboard/enable SSH/apt update etc.
3. Then took SD card out and put into Pi Zero W and booted.
4. SSHed into Pi and ran the following (after which the pi will power off):

```sh
curl -sSL https://raw.githubusercontent.com/raspberrypisig/pizero-usb-hid-keyboard/master/install.sh | sudo bash -
```
5. When pi is off, remove power supply and use an ORDINARY(not OTG cable) microUSB to USB cable and plug it in to the USB connector marked
USB on the board(the one next to the HDMI connector). Plug other end to Windows PC.
6. Be patient, eventually Windows sees it as a Generic USB keyboard (ignore device malformed warnings)
7. On Windows, open notepad
8. Using another computer, I ssh'd into pi and ran

```sh
cd pizero-usb-hid-keyboard
echo 'left-shift h' | ./hid_gadget_test /dev/hidg0 keyboard
echo 'i' | ./hid_gadget_test /dev/hidg0 keyboard
```

or alternatively
```sh
/home/pi/pizero-usb-hid-keyboard/sendkeys left-shift h
/home/pi/pizero-usb-hid-keyboard/sendkeys i
```

9. Success! Should see **Hi** in notepad
10. Look at https://github.com/raspberrypisig/pizero-usb-hid-keyboard/blob/master/hid-gadget-test.c#L20 for what is possible.

---

## Pi 3 B+ with Teensy 2.0++ (UART to USB HID)

The Pi 3 B+ lacks USB gadget capability, so this setup uses a **Teensy 2.0++** as the USB HID device. The Pi 3 sends keystroke commands to the Teensy over **UART (GPIO serial)**, and the Teensy sends them as USB keyboard input to the target PC.

```
User SSH --> Pi 3 B+ --> UART (GPIO14/15) --> Teensy 2.0++ --> USB HID --> Target PC
```

### Wiring

| Pi 3 B+ | Teensy 2.0++ | Notes |
|----------|-------------|-------|
| GPIO14/TXD (pin 8) | Pin D2 (Serial1 RX) | Direct connection OK |
| GPIO15/RXD (pin 10) | Pin D3 (Serial1 TX) | **Needs voltage divider** (see below) |
| GND (pin 6) | GND | Common ground |

**Voltage warning:** Pi 3 GPIO is 3.3V, Teensy 2.0++ is 5V. The Teensy TX (5V) to Pi RX line needs a voltage divider: **1K resistor** from Teensy TX, **2K resistor** to GND, midpoint to Pi RX. Pi TX (3.3V) to Teensy RX works directly (3.3V exceeds AVR logic threshold).

```
Teensy TX (D3) ---[1K]---+--- Pi RX (GPIO15)
                          |
                        [2K]
                          |
                         GND
```

### Flash Teensy Firmware

1. Install [Arduino IDE](https://www.arduino.cc/en/software) + [Teensyduino](https://www.pjrc.com/teensy/td_download.html)
2. Open `teensy_firmware/teensy_hid.ino`
3. Select Board: **Teensy++ 2.0**
4. Select USB Type: **Keyboard**
5. Click Upload

### Pi 3 Setup

SSH into the Pi 3 and run:

```sh
git clone https://github.com/raspberrypisig/pizero-usb-hid-keyboard.git
cd pizero-usb-hid-keyboard
sudo bash install-pi3.sh
sudo reboot
```

### Testing

After reboot, connect the Teensy USB to the target PC, open a text editor on the target, then from the Pi 3:

```sh
cd pizero-usb-hid-keyboard
echo 'h i' | ./hid-gadget-test /dev/serial0 keyboard
```

Or use the `sendkeys` helper:

```sh
./sendkeys h i
```

Should type "hi" on the target PC.

### Protocol

The Pi sends framed packets over UART at 115200 baud:
- `[0xFD][8-byte HID report]` (9 bytes total)
- Teensy applies the HID report and replies with ACK byte `0xAC`
