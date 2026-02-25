#!/usr/bin/env bash

# Please run this as root

cp /etc/rc.local /etc/rc.local.bak

if grep -q "^exit 0" /etc/rc.local 2>/dev/null; then
  sed -i '/^exit 0/i /home/pi/pizero-usb-hid-keyboard/rpi-hid.sh\nchmod 777 /dev/hidg0\n' /etc/rc.local
else
  cat<<EOF >> /etc/rc.local
/home/pi/pizero-usb-hid-keyboard/rpi-hid.sh
chmod 777 /dev/hidg0
exit 0
EOF
fi

chmod 755 /etc/rc.local
