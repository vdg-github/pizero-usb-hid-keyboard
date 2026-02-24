/*
 * Teensy 2.0++ USB HID Keyboard Firmware
 *
 * Receives 8-byte HID keyboard reports over Serial1 (UART, pins D2/D3)
 * and sends them as USB HID keypresses to the connected PC.
 *
 * Protocol:
 *   - Framed packets: [0xFD][8 bytes HID report] (9 bytes total)
 *   - Sends ACK byte (0xAC) after processing each report
 *
 * Wiring (Pi 3 B+ to Teensy 2.0++):
 *   Pi GPIO14/TXD (pin 8)  --> Teensy Pin D2 (Serial1 RX)  [direct]
 *   Pi GPIO15/RXD (pin 10) <-- Teensy Pin D3 (Serial1 TX)  [voltage divider: 1K + 2K]
 *   Pi GND (pin 6)         --- Teensy GND
 *
 * Board: Teensy++ 2.0
 * USB Type: Keyboard (in Arduino IDE / Teensyduino)
 */

#define FRAME_BYTE 0xFD
#define ACK_BYTE   0xAC
#define BAUD_RATE  115200
#define REPORT_LEN 8

void setup() {
  Serial1.begin(BAUD_RATE);
}

void loop() {
  static uint8_t report[REPORT_LEN];
  static uint8_t idx = 0;
  static bool synced = false;

  while (Serial1.available()) {
    uint8_t b = Serial1.read();

    if (!synced) {
      if (b == FRAME_BYTE) {
        synced = true;
        idx = 0;
      }
      continue;
    }

    report[idx++] = b;

    if (idx == REPORT_LEN) {
      // Apply the HID report
      Keyboard.set_modifier(report[0]);
      // report[1] is reserved (always 0)
      Keyboard.set_key1(report[2]);
      Keyboard.set_key2(report[3]);
      Keyboard.set_key3(report[4]);
      Keyboard.set_key4(report[5]);
      Keyboard.set_key5(report[6]);
      Keyboard.set_key6(report[7]);
      Keyboard.send_now();

      // Send ACK
      Serial1.write(ACK_BYTE);

      synced = false;
      idx = 0;
    }
  }
}
