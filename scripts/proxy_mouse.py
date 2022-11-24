# Cross-platform proxy variant
# Based loosely on https://github.com/arjunr2/logitech-wheel-dev

import ctypes
import struct
import sys
import time
import pygame

from uart_interface import *

UPDATE_INTERVAL_MS = 50
PORT = "/dev/tty.usbserial-ABSCEYPL"

iface = UARTWheelInterface(PORT)
iface.start()

screen = pygame.display.set_mode((400, 400))
pygame.display.set_caption("Steering Wheel")

while True:
    pygame.time.wait(10)
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            sys.exit(0)

    x, y = 200, 200

    if pygame.mouse.get_focused():
        x, y = pygame.mouse.get_pos()
        if y > 185:
            y = 200
        if abs(x-200) < 15:
            x = 200

        steer = clamp(int((x - 200) / 2), -100, 100)
        throttle = clamp(int((200 - y) / 2), 0, 100)

        BL = pygame.key.get_pressed()[pygame.K_LEFT]
        BR = pygame.key.get_pressed()[pygame.K_RIGHT]

        A = pygame.key.get_pressed()[pygame.K_a]
        B = pygame.key.get_pressed()[pygame.K_b]
        X = pygame.key.get_pressed()[pygame.K_x]
        Y = pygame.key.get_pressed()[pygame.K_y]

        brake = pygame.key.get_pressed()[pygame.K_SPACE]

        iface.set_state(steer, throttle, BL, BR, A, B, X, Y, brake)
    else:
        iface.set_state(0, 0, 0, 0, 0, 0, 0, 0, 0)


    screen.fill((0, 0, 0))

    pygame.draw.line(screen, (255, 255, 255), (0, 200), (400, 200), width=2)
    pygame.draw.line(screen, (255, 255, 255), (200, 0), (200, 400), width=2)

    for i in range(40, 400, 40):
        pygame.draw.line(screen, (255, 255, 255), (i, 190), (i, 210), width=1)
        pygame.draw.line(screen, (255, 255, 255), (190, i), (210, i), width=1)

    pygame.draw.circle(screen, (0, 200, 0), (x, y), 10)

    pygame.draw.circle(screen, (220, 0, 0), (200 + 2*int(iface.get_feedback()), 390), 10)

    pygame.display.flip()



"""
state_c = lsw.get_c_state(0)
lsw.update()
state = lsw.get_state(0)
btns = [i for i, x in enumerate(state.rgbButtons) if x != 0]

steer = clamp(int(state.lX / 100), -100, 100)
throttle = clamp(int((-state.lY + 32768) / 655.36), 0, 100)
if throttle < 5: throttle = 0
brake = (state.lRz < 30000)

self.iface.set_state(steer, throttle, (5 in btns), (4 in btns),
        (0 in btns), (1 in btns), (2 in btns), (3 in btns), brake)

lsw.play_constant_force(0, int(self.iface.get_feedback()))
"""
