from hitl_test import *
from time import sleep

# make it easier to set only relevant values on interface
def enc(BL, BR, steer):
    return [steer, 0, BL, BR, 0, 0, 0, 0, 0]

class TestPower(HITLTestBase):
    def setup(self): pass

    def run_test(self):
        self.prompt("ensure that the DUT is powered on")
        self.expect("'Power' LED on each of the three zone boards is solid green")

    def teardown(self): pass

class TestLeftBlinkerEnableWithButton(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

    def run_test(self):
        print("Pressing BL button for one second")
        self.iface.set_state(*enc(BL=1, BR=0, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)
        self.expect("left blinker is blinking at 1 Hz")

class TestRightBlinkerEnableWithButton(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

    def run_test(self):
        print("Pressing BR button for one second")
        self.iface.set_state(*enc(BL=0, BR=1, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)
        self.expect("right blinker is blinking at 1 Hz")

class TestLeftBlinkerDisableWithButton(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

        print("Pressing BL button for one second")
        self.iface.set_state(*enc(BL=1, BR=0, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)

        self.expect("left blinker is blinking at 1 Hz")

    def run_test(self):
        print("Pressing BL button for one second")
        self.iface.set_state(*enc(BL=1, BR=0, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)
        self.expect("all blinkers are off")

class TestRightBlinkerDisableWithButton(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

        print("Pressing BR button for one second")
        self.iface.set_state(*enc(BL=0, BR=1, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)

        self.expect("right blinker is blinking at 1 Hz")

    def run_test(self):
        print("Pressing BR button for one second")
        self.iface.set_state(*enc(BL=0, BR=1, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)
        self.expect("all blinkers are off")

class TestLeftBlinkerHysteresis(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

        print("Pressing BL button for one second")
        self.iface.set_state(*enc(BL=1, BR=0, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)

        self.expect("left blinker is blinking at 1 Hz")

    def run_test(self):
        print("Rotating steering to position -40")
        self.iface.set_state(*enc(BL=0, BR=0, steer=-40))
        sleep(1)
        self.expect("left blinker is blinking at 1 Hz")

        print("Rotating steering to position 0")
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)
        self.expect("left blinker is blinking at 1 Hz")

class TestRightBlinkerHysteresis(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

        print("Pressing BR button for one second")
        self.iface.set_state(*enc(BL=0, BR=1, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)

        self.expect("right blinker is blinking at 1 Hz")

    def run_test(self):
        print("Rotating steering to position 40")
        self.iface.set_state(*enc(BL=0, BR=0, steer=40))
        sleep(1)
        self.expect("right blinker is blinking at 1 Hz")

        print("Rotating steering to position 0")
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)
        self.expect("right blinker is blinking at 1 Hz")

class TestLeftBlinkerThreshold(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

        print("Pressing BL button for one second")
        self.iface.set_state(*enc(BL=1, BR=0, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)

        self.expect("left blinker is blinking at 1 Hz")

    def run_test(self):
        print("Rotating steering to position -80")
        self.iface.set_state(*enc(BL=0, BR=0, steer=-80))
        sleep(1)
        self.expect("left blinker is blinking at 1 Hz")

        print("Rotating steering to position -50")
        self.iface.set_state(*enc(BL=0, BR=0, steer=-50))
        sleep(1)
        self.expect("left blinker is blinking at 1 Hz")

        print("Rotating steering to position 0")
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)
        self.expect("all blinkers are off")

class TestRightBlinkerThreshold(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

        print("Pressing BR button for one second")
        self.iface.set_state(*enc(BL=0, BR=1, steer=0))
        sleep(1)
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)

        self.expect("right blinker is blinking at 1 Hz")

    def run_test(self):
        print("Rotating steering to position 80")
        self.iface.set_state(*enc(BL=0, BR=0, steer=80))
        sleep(1)
        self.expect("right blinker is blinking at 1 Hz")

        print("Rotating steering to position 50")
        self.iface.set_state(*enc(BL=0, BR=0, steer=50))
        sleep(1)
        self.expect("right blinker is blinking at 1 Hz")

        print("Rotating steering to position 0")
        self.iface.set_state(*enc(BL=0, BR=0, steer=0))
        sleep(1)
        self.expect("all blinkers are off")

class TestSteeringZoneFailure(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

    def run_test(self):
        self.prompt("press self-test button for steering zone")
        self.expect("'Power' LED on steering zone is off")
        self.expect("'Status' LED on cockpit and drivetrain zones is solid orange")
        self.expect("both front blinkers are off")
        self.expect("both rear blinkers are flashing rapidly")

        self.prompt("press self-test button for steering zone")
        sleep(1)
        self.expect("'Power' LED on steering zone is on")
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

class TestDrivetrainZoneFailure(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

    def run_test(self):
        self.prompt("press self-test button for drivetrain zone")
        self.expect("'Power' LED on drivetrain zone is off")
        self.expect("'Status' LED on cockpit and steering zones is solid orange")
        self.expect("both rear blinkers are off")
        self.expect("both front blinkers are flashing rapidly")

        self.prompt("press self-test button for steering zone")
        sleep(1)
        self.expect("'Power' LED on steering zone is on")
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

class TestCockpitZoneFailure(HITLTestBase):
    def setup(self):
        # because blinker statefulness is in the cockpit zone,
        # resetting it should clean up the system state
        self.prompt("press reset button on cockpit zone")
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

    def run_test(self):
        print("Setting steering wheel to send corrupt packets")
        self.iface.send_bad_data = True
        sleep(1)
        self.expect("'Status' LED on drivetrain and steering zones is solid orange")
        self.expect("all blinkers are flashing rapidly")

        print("Setting steering wheel to send good packets")
        self.iface.send_bad_data = False
        sleep(1)
        self.expect("'Status' LEDs on each of the three zone boards is off")
        self.expect("all blinkers are off")

if __name__ == "__main__":
    run_tests([
        TestPower,
        TestLeftBlinkerEnableWithButton,
        TestRightBlinkerEnableWithButton,
        TestLeftBlinkerDisableWithButton,
        TestRightBlinkerDisableWithButton,
        TestLeftBlinkerHysteresis,
        TestRightBlinkerHysteresis,
        TestLeftBlinkerThreshold,
        TestRightBlinkerThreshold,
        TestSteeringZoneFailure,
        TestDrivetrainZoneFailure,
        TestCockpitZoneFailure
    ])
