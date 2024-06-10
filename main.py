#!venv/bin/python3

import cv2
import socket
import struct
import io
import numpy as np

def main(ip, port):
    video = cv2.VideoCapture(0)
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((ip, port))
    while True:
        req, remote_host = s.recvfrom(65536)

        if req == 'bdsc?':
            # Remote discovery
            s.sendto(b'dsc+', remote_host)
            continue

        if req != b'jpg?':
            continue

        # Handle request for JPEG camera frame.

        ret, img = video.read()
        if not ret:
            # Error occurred while grabbing image frame.
            s.sendto(b'jpg-', remote_host)
            continue

        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 25]
        img_buffer = np.array(cv2.imencode('.jpg', img, encode_param)[1]).tobytes()
        if len(img_buffer) > 32768:
            # Image size is too large.
            s.sendto(b'jpg-', remote_host)
            continue
            
        response = bytearray()
        response += b'jpg+'
        response += img_buffer

        s.sendto(bytes(response), remote_host)

    video.release()
    s.close()

main('192.168.1.42', 6255)
