

export PORT=cu.usbserial-1420

stty 115200 < /dev/$PORT

echo -ne 'set short name shell' > /dev/$PORT

#cat -v < /dev/cu.usbserial-1420

# screen /dev/cu.usbserial-1420

#  better : @type file.txt > COM5