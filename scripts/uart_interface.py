import threading
import time
import serial

UPDATE_TIME_MS = 50

def clamp(n, _min, _max):
    return max(_min, min(n, _max))

class UARTWheelInterface:
    def __init__(self, port, verbose=True):
        self.steering = 0
        self.throttle = 0
        self.btns = 0
        self.feedback = 0
        self.verbose = verbose

        self.s = serial.Serial(port, baudrate=115200)

        self.send_bad_data = False

    def set_state(self, steering, throttle, BL, BR, A, B, X, Y, brake):
        self.steering = clamp(steering, -100, 100)
        self.throttle = clamp(throttle, 0, 100)

        self.btns = (0x80 if BL else 0x00) | \
            (0x40 if BR else 0x00) | \
            (0x20 if A else 0x00) | \
            (0x10 if B else 0x00) | \
            (0x08 if X else 0x00) | \
            (0x04 if Y else 0x00) | \
            (0x02 if brake else 0x00)

    def get_feedback(self):
        return self.feedback

    def _update(self):
        while self.s.in_waiting > 0:
            self.feedback = self.s.read(1)[0]
            if self.feedback > 127:
                self.feedback = self.feedback - 256

            if self.verbose:
                print("[UART] RX feedback", self.feedback)

        s = self.steering
        if s < 0: s = 256 + s
        chk = (s + self.throttle + self.btns) & 0xFF
        if self.send_bad_data: chk = chk + 1
        self.s.write(bytes([0x01, s, self.throttle, self.btns, chk]))

        if self.verbose:
            print("[UART] TX update", self.steering, self.throttle,
                  "{:08b}".format(self.btns))

    def _loop(self):
        while True:
            self._update()
            time.sleep(UPDATE_TIME_MS / 1000.0)

    def start(self):
        self.thread = threading.Thread(target=self._loop, daemon=True)
        self.thread.start()
