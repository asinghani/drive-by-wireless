# Logitech Steering Wheel Proxy
# Windows-only
# Based loosely on https://github.com/arjunr2/logitech-wheel-dev

from asyncio.proactor_events import _ProactorDuplexPipeTransport
import ctypes
import struct
import sys
import time

from PyQt5 import QtGui, QtWidgets, QtCore
import logitech_steering_wheel as lsw

from uart_interface import *

UPDATE_INTERVAL_MS = 50
PORT = "COM5"

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self, iface):
        super().__init__()

        self.iface = iface

        self.update_timer = QtCore.QTimer()
        self.update_timer.setInterval(UPDATE_INTERVAL_MS)
        self.update_timer.setSingleShot(False)
        self.update_timer.timeout.connect(self._update_sw)

        self.connect_button = QtWidgets.QPushButton("connect")
        self.stop_button = QtWidgets.QPushButton("quit")

        self.connect_button.clicked.connect(self.connect_to_wheel)
        self.stop_button.clicked.connect(self.stop)

        layout = QtWidgets.QVBoxLayout()
        widget = QtWidgets.QWidget()
        widget.setLayout(layout)

        layout.addWidget(self.connect_button)
        layout.addWidget(self.stop_button)

        self.setCentralWidget(widget)

    def connect_to_wheel(self) -> None:
        initialized = lsw.initialize(True)

        if initialized:
            print("initialized successfully")
        else:
            print("failed")
            sys.exit(1)

        if lsw.is_connected(0):
            print("connected to a steering wheel at index 0")
        else:
            print("failed to connect")
            sys.exit(1)

        lsw.update()

        # Move wheel to register initial state
        lsw.play_bumpy_road_effect(0, 20)
        time.sleep(0.04)
        lsw.stop_bumpy_road_effect(0)

        self.update_timer.start()
        print(lsw.get_current_controller_properties(0))

    def _update_sw(self):
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

    def stop(self):
        print("Disconnect")
        lsw.shutdown()
        self.update_timer.stop()
        sys.exit(0)

    def closeEvent(self, a0: QtGui.QCloseEvent) -> None:
        self.stop()

if __name__ == "__main__":
    iface = UARTWheelInterface(PORT)
    iface.start()

    app = QtWidgets.QApplication(sys.argv)
    window = MainWindow(iface)
    window.show()
    sys.exit(app.exec_())
