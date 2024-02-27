#!/usr/bin/env python

from gpiozero import OutputDevice
from time import sleep

o = OutputDevice(21)


while True:
    o.on()
    sleep(1)
    o.off()
    sleep(1)
