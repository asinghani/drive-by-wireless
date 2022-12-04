from uart_interface import *
import sys
from colorama import just_fix_windows_console
from termcolor import colored
just_fix_windows_console()

class HITLTestBase:
    def __init__(self, iface):
        self.success = True
        self.iface = iface

    def _run_test(self):
        test_name = type(self).__name__
        print(colored("="*80, attrs=["bold"]))
        print(colored(f"Starting test {test_name}...", "yellow", attrs=["bold"]))
        print(colored("="*80, attrs=["bold"]))
        self.setup()
        if not self.success:
            print(colored("Preconditions failed. Skipping test.", "red"))
        else:
            self.run_test()
            self.teardown()

        print(colored("="*80, attrs=["bold"]))
        if self.success: print(colored(f"Test {test_name} successful", "green", attrs=["bold"]))
        else: print(colored(f"Test {test_name} failed", "red", attrs=["bold"]))
        print(colored("="*80, attrs=["bold"]))
        print()
        print()
        return test_name, self.success

    def prompt(self, instruction):
        input(f"Please perform the following action on the DUT: '{colored(instruction, attrs=['bold'])}'. Press enter when finished... ")

    def expect(self, instruction, expected=True):
        res = None
        while True:
            x = input(f"Is the following condition met: '{colored(instruction, attrs=['bold'])}'? (Y/n) ").lower()

            res = None
            if x in ["y", "yes", "true"]:
                res = True

            if x in ["n", "no", "false"]:
                res = False

            if res is not None:
                break

        if res != expected:
            print(f"Error: Expected '{instruction}' to be {expected}, got {res}")
            self.success = False

    # Functions to override
    def setup(self):
        pass # optional

    def run_test(self):
        raise NotImplementedError("run_test() is required")

    def teardown(self):
        pass # optional

def run_tests(tests):
    iface = UARTWheelInterface(sys.argv[1], False)
    iface.start()

    succ_tests = []
    fail_tests = []
    for test in tests:
        t = test(iface)
        name, succ = t._run_test()
        if succ:
            succ_tests.append(name)
        else:
            fail_tests.append(name)

    ns = len(succ_tests)
    nf = len(fail_tests)
    n = ns+nf

    print(colored("="*80, attrs=["bold"]))
    if ns == n:
        print(colored(f"    All {n} tests successful", "green", attrs=["bold"]))
    else:
        print(colored(f"    {nf} out of {n} tests failed", "red", attrs=["bold"]))
    print(colored("="*80, attrs=["bold"]))
