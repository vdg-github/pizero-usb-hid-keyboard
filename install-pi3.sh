#!/bin/bash
# Install script for Pi 3 B+ with Teensy 2.0++ USB HID setup
# This configures the UART and compiles hid-gadget-test for serial mode.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=== Pi 3 B+ + Teensy 2.0++ HID Keyboard Setup ==="

# Enable UART
CONFIG="/boot/firmware/config.txt"
if [ ! -f "$CONFIG" ]; then
  CONFIG="/boot/config.txt"
fi

if ! grep -q "^enable_uart=1" "$CONFIG" 2>/dev/null; then
  echo "Enabling UART in $CONFIG..."
  echo "enable_uart=1" | sudo tee -a "$CONFIG" > /dev/null
else
  echo "UART already enabled in $CONFIG"
fi

# Disable serial console on ttyS0
echo "Disabling serial console on ttyS0..."
sudo systemctl disable serial-getty@ttyS0.service 2>/dev/null || true
sudo systemctl stop serial-getty@ttyS0.service 2>/dev/null || true

# Remove console=serial from cmdline.txt
CMDLINE="/boot/firmware/cmdline.txt"
if [ ! -f "$CMDLINE" ]; then
  CMDLINE="/boot/cmdline.txt"
fi

if grep -q "console=serial0" "$CMDLINE" 2>/dev/null; then
  echo "Removing serial console from $CMDLINE..."
  sudo sed -i 's/console=serial0,[0-9]* //g' "$CMDLINE"
fi
if grep -q "console=ttyS0" "$CMDLINE" 2>/dev/null; then
  sudo sed -i 's/console=ttyS0,[0-9]* //g' "$CMDLINE"
fi
if grep -q "console=ttyAMA0" "$CMDLINE" 2>/dev/null; then
  sudo sed -i 's/console=ttyAMA0,[0-9]* //g' "$CMDLINE"
fi

# Compile hid-gadget-test
echo "Compiling hid-gadget-test..."
cd "$SCRIPT_DIR"
gcc -o hid-gadget-test hid-gadget-test.c -lpthread
echo "Compiled hid-gadget-test successfully."

echo ""
echo "=== Setup complete ==="
echo "Reboot required for UART changes to take effect."
echo "After reboot, test with:"
echo "  echo 'h i' | $SCRIPT_DIR/hid-gadget-test /dev/serial0 keyboard"
echo "  $SCRIPT_DIR/sendkeys h i"
