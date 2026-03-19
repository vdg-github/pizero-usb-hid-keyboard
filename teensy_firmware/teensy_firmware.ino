/*
 * Teensy 2.0++ USB HID Keyboard + Mouse Firmware
 *
 * Receives HID reports over Serial1 (UART, pins D2/D3) and sends them
 * as USB HID keypresses or mouse movements to the connected device.
 *
 * Protocol:
 *   - Keyboard: [0xFD][8 bytes HID report] (9 bytes total)
 *   - Mouse:    [0xFE][buttons][dx][dy]    (4 bytes total)
 *   - Sends ACK byte (0xAC) after processing each report
 *
 * Wiring (Pi 3 B+ to Teensy 2.0++):
 *   Pi GPIO14/TXD (pin 8)  --> Teensy Pin D2 (Serial1 RX)  [direct]
 *   Pi GPIO15/RXD (pin 10) <-- Teensy Pin D3 (Serial1 TX)  [voltage divider: 1K + 2K]
 *   Pi GND (pin 6)         --- Teensy GND
 *
 * Board: Teensy++ 2.0
 * USB Type: Keyboard + Mouse + Joystick (usb=hid)
 */

#define FRAME_KB   0xFD
#define FRAME_MOUSE 0xFE
#define ACK_BYTE   0xAC
#define BAUD_RATE  115200
#define KB_REPORT_LEN    8
#define MOUSE_REPORT_LEN 3

void setup() {
  Serial1.begin(BAUD_RATE);
}

void loop() {
  static uint8_t buf[KB_REPORT_LEN]; // big enough for either report type
  static uint8_t idx = 0;
  static uint8_t frame_type = 0;
  static uint8_t expected_len = 0;

  while (Serial1.available()) {
    uint8_t b = Serial1.read();

    if (frame_type == 0) {
      // Waiting for frame byte
      if (b == FRAME_KB) {
        frame_type = FRAME_KB;
        expected_len = KB_REPORT_LEN;
        idx = 0;
      } else if (b == FRAME_MOUSE) {
        frame_type = FRAME_MOUSE;
        expected_len = MOUSE_REPORT_LEN;
        idx = 0;
      }
      continue;
    }

    buf[idx++] = b;

    if (idx == expected_len) {
      if (frame_type == FRAME_KB) {
        // Apply keyboard HID report
        Keyboard.set_modifier(buf[0]);
        // buf[1] is reserved (always 0)
        Keyboard.set_key1(buf[2]);
        Keyboard.set_key2(buf[3]);
        Keyboard.set_key3(buf[4]);
        Keyboard.set_key4(buf[5]);
        Keyboard.set_key5(buf[6]);
        Keyboard.set_key6(buf[7]);
        Keyboard.send_now();
      } else if (frame_type == FRAME_MOUSE) {
        // Apply mouse report: [buttons][dx][dy]
        uint8_t buttons = buf[0];
        int8_t dx = (int8_t)buf[1];
        int8_t dy = (int8_t)buf[2];
        Mouse.set_buttons(buttons & 0x01, (buttons >> 1) & 0x01, (buttons >> 2) & 0x01);
        Mouse.move(dx, dy);
      }

      // Send ACK
      Serial1.write(ACK_BYTE);

      frame_type = 0;
      idx = 0;
    }
  }
}
