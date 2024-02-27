#!/usr/bin/env python

from gpiozero import LED

led = LED(21)
led.blink(background=False)
