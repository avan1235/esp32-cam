import websocket
import requests
import numpy as np
import cv2

OPCODE_TEXT = 0x1
OPCODE_BIN = 0x2
OPCODE_PING = 0x9
OPCODE_PONG = 0xa


class WsClient:
    def __init__(self, ip, port, path):
        self.port = port
        self.ip = ip
        self.path = path
        self.ws = websocket.WebSocket()

    def __enter__(self):
        self.ws.connect(f"ws://{self.ip}:{self.port}{self.path}")
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.ws.close()

    def read(self):
        try:
            return self.ws.recv_data(control_frame=False)
        except:
            return None, None

    def write(self, data, opcode=OPCODE_TEXT):
        if opcode == OPCODE_BIN:
            return self.ws.send_binary(data.encode())
        if opcode == OPCODE_PING:
            return self.ws.ping(data)
        return self.ws.send(data)


class Camera:
    def __init__(self, camera_ip="192.168.4.1", recv_port=80):
        def control_url(path):
            requests.put(f"http://{camera_ip}:{recv_port}{path}")

        self.control = control_url
        self.video = WsClient(camera_ip, recv_port, "/video")

    def __enter__(self):
        self.video = self.video.__enter__()
        return self

    def __exit__(self, *args, **kwargs):
        self.video.__exit__(*args, **kwargs)

    def get_frame(self):
        self.video.write("s")
        _, data = self.video.read()
        if data is None:
            return None
        array = np.frombuffer(data, dtype=np.uint8)
        return cv2.imdecode(array, cv2.IMREAD_UNCHANGED)

    def increase_quality(self):
        self.control("/control/resolution/increase")

    def decrease_quality(self):
        self.control("/control/resolution/decrease")

    def switch_flash_led(self):
        self.control("/control/led/switch")
