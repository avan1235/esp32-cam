import websocket

OPCODE_TEXT = 0x1
OPCODE_BIN = 0x2
OPCODE_PING = 0x9
OPCODE_PONG = 0xa


class WsClient:
    def __init__(self, ip: str, port: int) -> None:
        self.port = port
        self.ip = ip
        self.ws = websocket.WebSocket()

    def __enter__(self):    # type: ignore
        self.ws.connect('ws://{}:{}/ws'.format(self.ip, self.port))
        return self

    def __exit__(self, exc_type, exc_value, traceback):     # type: ignore
        self.ws.close()

    def read(self):     # type: ignore
        return self.ws.recv_data(control_frame=True)

    def write(self, data='', opcode=OPCODE_TEXT):   # type: ignore
        if opcode == OPCODE_BIN:
            return self.ws.send_binary(data.encode())
        if opcode == OPCODE_PING:
            return self.ws.ping(data)
        return self.ws.send(data)


if __name__ == "__main__":
    ip = "192.168.4.1"
    port = 80
    with WsClient(ip, port) as ws:
        ws.write("Trigger async", opcode=OPCODE_TEXT)
        print(ws.read())
