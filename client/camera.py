import websocket
import requests
import numpy as np
import cv2

OPCODE_TEXT = 0x1
OPCODE_BIN = 0x2
OPCODE_PING = 0x9
OPCODE_PONG = 0xa


class Camera:
    def __init__(self, ip="192.168.4.1", port=80):
        def control_url(path):
            requests.put(f"http://{ip}:{port}/control{path}")

        self.control = control_url
        self.ws_url = f"ws://{ip}:{port}/video"
        self.ws = websocket.WebSocket()

    def __enter__(self):
        self.ws.connect(self.ws_url)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.ws.close()

    def get_frame(self):
        self.__ws_write("s")
        _, data = self.__ws_read()
        if data is None:
            return None
        array = np.frombuffer(data, dtype=np.uint8)
        return cv2.imdecode(array, cv2.IMREAD_UNCHANGED)

    def increase_quality(self):
        self.control("/resolution/increase")

    def decrease_quality(self):
        self.control("/resolution/decrease")

    def switch_flash_led(self):
        self.control("/led/switch")

    def __ws_write(self, data, opcode=OPCODE_TEXT):
        if opcode == OPCODE_BIN:
            return self.ws.send_binary(data.encode())
        if opcode == OPCODE_PING:
            return self.ws.ping(data)
        return self.ws.send(data)

    def __ws_read(self):
        try:
            return self.ws.recv_data()
        except:
            return None, None
