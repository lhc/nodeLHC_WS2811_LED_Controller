import socket
import time
import random
import pygame
import pygame.camera
import pygame.transform
from pygame.locals import *
import sys

import binascii

sequence = 0x00 # Sequence == 0 disable this feature
universe = 0x00 

x_len = 8
y_len = 8
length = x_len*y_len
len_msb = (length >> 8) & 0xff
len_lsb = (length) & 0xff

UDP_IP = "192.168.15.14"
#UDP_PORT = 0x1936 #(6454 decimal)
UDP_PORT = 6454

pygame.init()
pygame.camera.init()

camlist = pygame.camera.list_cameras()
if camlist:
    cam = pygame.camera.Camera(camlist[0],(640,480), "RGB")
    cam.start()
else:
    raise "Sem camera"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def yield_bytes(surf):
    for y in range(y_len):
        for x in range(x_len):
            color = image.get_at((x, y))
            yield chr(color.g)
            yield chr(color.r)
            yield chr(color.b)

while True:
        image = pygame.transform.smoothscale(cam.get_image(), (x_len, y_len))
        message = bytes('Art-Net' + '\x00\x00\x50\x00\x0e' + chr(sequence) + '\x00' + 
                chr(universe) + '\x00' + chr(len_msb) + 
                chr(len_lsb) + 
                ''.join(yield_bytes(image)), 'iso-8859-1')
        sock.sendto(message, (UDP_IP, UDP_PORT))
        sequence += 1
        if sequence == 256:
            sequence = 1

