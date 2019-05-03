import socket
import time
import random
import pygame
import pygame.camera
import pygame.transform
from pygame.locals import *

UDP_IP = "192.168.15.14"
UDP_PORT = 2711

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
	for x in range(10):
		for y in range(10):
			color = image.get_at((x, y))
			yield chr(color.r)
			yield chr(color.g)
			yield chr(color.b)

while True:
	image = pygame.transform.smoothscale(cam.get_image(), (10, 10))
	message = bytes('\x00\x00\x00\x00\x64' + ''.join(yield_bytes(image)), 'iso-8859-1')
	sock.sendto(message, (UDP_IP, UDP_PORT))

