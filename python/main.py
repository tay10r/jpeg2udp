#!venv/bin/python3

import cv2
import socket
import struct
import io
import numpy as np
import argparse
from loguru import logger

QUALITY = 25 # TODO : add this to the stream request
BW = True    # TODO : add this to the stream request

def run_io_loop(ip: str, port: int, enable_logging: bool):
    video = cv2.VideoCapture(0)
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((ip, port))
    if not enable_logging:
        logger.remove()
    while True:
        req, remote_host = s.recvfrom(65536)

        if req == b'dsc?':
            # Remote discovery
            logger.info(f'Received discovery request from "{remote_host}".')
            s.sendto(b'dsc+', remote_host)
            continue

        if req != b'jpg?':
            logger.error(f'Unknown request from "{remote_host}".')
            continue

        # Handle request for JPEG camera frame.
        logger.info(f'Received JPEG request from "{remote_host}".')

        ret, img = video.read()
        if not ret:
            # Error occurred while grabbing image frame.
            s.sendto(b'jpg-', remote_host)
            logger.error('Error occurred while grabbing the latest camera frame.')
            continue

        if BW:
            img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), QUALITY]
        img_buffer = np.array(cv2.imencode('.jpg', img, encode_param)[1]).tobytes()
        if len(img_buffer) > 32768:
            # Image size is too large.
            s.sendto(b'jpg-', remote_host)
            logger.error('Frame is too large to encode in the UDP message.')
            continue
            
        response = bytearray()
        response += b'jpg+'
        response += img_buffer

        s.sendto(bytes(response), remote_host)

    video.release()
    s.close()

def main():
    parser = argparse.ArgumentParser('jpeg2udp', description='Streams imagery over UDP.')
    parser.add_argument('--bind-ip', default='0.0.0.0', help='The IPv4 address to bind to.', type=str)
    parser.add_argument('--bind-port', default=6255, help='The UDP port to bind to.', type=int)
    parser.add_argument('--enable-logging', default=False, help='Whether or not to enable logging.', type=bool)
    args = parser.parse_args()
    run_io_loop(args.bind_ip, args.bind_port, args.enable_logging)

if __name__ == '__main__':
    main()
