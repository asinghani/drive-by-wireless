#
# Copyright (c) 2019 Analog Devices Inc.
#
# This file is part of libm2k
# (see http://www.github.com/analogdevicesinc/libm2k).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

# This example will generate a binary counter on the first N_BITS of the
# digital interface and read them back - no additional connection required

import libm2k

# getNextEdge in data
def getNextEdge(data, pin, time, freq):
    target = (time * freq) / 1000000
    count = 0
    prev_pin_value = 0
    
    for val in data:
        high = val & (1 << pin)
        if (high):
            pin_value = 1
        else:
            pin_value = 0
        if count > target:
            if (prev_pin_value == 0 and pin_value == 1):
                return count * (1000000 / freq)
        prev_pin_value = pin_value
        count+=1
    
    return -1

# getNextEdge in data
def getPrevEdge(data, pin, time, freq):
    target = (time * freq) / 1000000
    count = 0
    prev_pin_value = 0
    time_found = -1
    
    for val in data:
        high = val & (1 << pin)
        if (high):
            pin_value = 1
        else:
            pin_value = 0

        if (count >= target):
            return time_found

        if (prev_pin_value == 0 and pin_value == 1):
            # This is an edge
            time_found = count * (1000000 / freq)

        prev_pin_value = pin_value
        count+=1
    
    return -1


# Count edges in sample
def countEdges(data, pin):
    prev_pin_value = 0
    count = 0

    for val in data:
        high = val & (1 << pin)
        if (high):
            pin_value = 1
        else:
            pin_value = 0
        if (prev_pin_value == 0 and pin_value == 1):
            # This is an edge
            count+=1
        prev_pin_value = pin_value
    
    return count

# edge_no starts at 0
def checkEdge(data, pin, edge_no, freq):
    prev_pin_value = 0
    current_edge = 0
    count = 0
    edge_found = 0

    for val in data:
        high = val & (1 << pin)
        if (high):
            pin_value = 1
        else:
            pin_value = 0
        if (prev_pin_value == 0 and pin_value == 1):
            # This is an edge
            if (current_edge == edge_no):
                edge_found = 1
                break
            else:
                current_edge+=1
        prev_pin_value = pin_value
        count+=1
    
    if (edge_found):
        return count * (1000000 / freq)
    else:
        return -1

# initiate adalm
# freq is in hertz
def init_adalm(freq):
    ctx=libm2k.m2kOpen()

    if ctx is None:
        print("Connection Error: No ADALM2000 device available/connected to your PC.")
        return -1
    
    dig=ctx.getDigital()
    dig.reset()
    dig.setSampleRateIn(freq)

    # Enable all digital channels
    for i in range(16):
        dig.setDirection(i,libm2k.DIO_INPUT)

    return dig

# Pin definition
CP_SEND = 1
CP_RECV = 2
ST_SEND = 3
ST_RECV = 4
DT_SEND = 5
DT_RECV = 6
ST_ERROR = 7
DT_ERROR = 8
VDD_ON = 9

# Initiate ADALM
freq = 100000
dig = init_adalm(100000)

if (dig == -1):
    exit()

print("\n\n")
print("*****************************************************")
print("***************** Testcase 1: ***********************")
print("*****************************************************\n")
print("Testing average round trip Latency, press enter to continue:")
x = input()

# Test for 1 second
num_samples = 100000

# Get samples from buffer
data = dig.getSamples(1)
dig.startAcquisition(num_samples)
data = dig.getSamples(num_samples)

# Process Steering Zone Receive
num_edges = countEdges(data, ST_RECV)
TOTAL_ST_RECV_TIME = 0
ST_count = 0

for i in range(5, num_edges - 5):
    # Get CP_SEND Time
    recv_time = checkEdge(data, ST_RECV, i, freq)
    send_time = getPrevEdge(data, CP_SEND, recv_time, freq)
    TOTAL_ST_RECV_TIME += (recv_time - send_time)
    ST_count += 1

if (ST_count != 0):
    AVG_ST_RECV_TIME = TOTAL_ST_RECV_TIME / ST_count
else:
    AVG_ST_RECV_TIME = 0


# Process Drivetrain Zone Receive
num_edges = countEdges(data, DT_RECV)
TOTAL_DT_RECV_TIME = 0
DT_count = 0

for i in range(5, num_edges - 5):
    # Get CP_SEND Time
    recv_time = checkEdge(data, DT_RECV, i, freq)
    send_time = getPrevEdge(data, CP_SEND, recv_time, freq)
    TOTAL_DT_RECV_TIME += (recv_time - send_time)
    DT_count += 1

if (DT_count != 0):
    AVG_DT_RECV_TIME = TOTAL_DT_RECV_TIME / DT_count
else:
    AVG_DT_RECV_TIME = 0


# Process Cockpit Zone Receive
num_edges = countEdges(data, CP_RECV)
TOTAL_CP_RECV_TIME = 0
CP_count = 0

for i in range(5, num_edges - 5):
    # Get CP_SEND Time
    recv_time = checkEdge(data, CP_RECV, i, freq)
    send_time1 = getPrevEdge(data, DT_SEND, recv_time, freq)
    send_time2 = getPrevEdge(data, ST_SEND, recv_time, freq)
    if (send_time1 > send_time2):
        TOTAL_CP_RECV_TIME += (recv_time - send_time1)
    else:
        TOTAL_CP_RECV_TIME += (recv_time - send_time2)
    CP_count += 1

if (CP_count != 0):
    AVG_CP_RECV_TIME = TOTAL_CP_RECV_TIME / CP_count
else:
    AVG_CP_RECV_TIME = 0

# Summary
total_count = CP_count + DT_count + ST_count
if (total_count != 0):
    AVG_TOTAL_TIME = (TOTAL_CP_RECV_TIME + TOTAL_DT_RECV_TIME + TOTAL_ST_RECV_TIME) / total_count
else:
    AVG_TOTAL_TIME = 0

print("\nTimings Obtained:")
print("Average Message to Steering Zone Time: " + str(AVG_ST_RECV_TIME) + "us")
print("Average Message to Drivetrain Zone Time: " + str(AVG_DT_RECV_TIME) + "us")
print("Average Message to Cockpit Zone Time: " + str(AVG_CP_RECV_TIME) + "us")
print("Average Message Time: " + str(AVG_TOTAL_TIME) + "us")

print("")
print("Press enter to continue...")
x = input()

print("")
print("*****************************************************")
print("***************** Testcase 2: ***********************")
print("*****************************************************\n")
print("Checking time taken to enter error state when steering zone fail")
print("Ensure that all three zones are currently functioning")
print("When you press enter, you have 10 seconds to kill the steering zone")
print("Press enter to continue:")
x = input()

# Test for 10 second
num_samples = 1000000

# Get samples from buffer
data = dig.getSamples(1)
dig.startAcquisition(num_samples)
data = dig.getSamples(num_samples)

# Process Drivetrain Zone Error
error_asserted = checkEdge(data, DT_ERROR, 0, freq)
DT_RECV_ERR_TIME = getPrevEdge(data, DT_RECV, error_asserted, freq)
CP_SEND_ERR_TIME = getPrevEdge(data, CP_SEND, DT_RECV_ERR_TIME, freq)
CP_FOUND_ERROR_TIME = getPrevEdge(data, CP_SEND, CP_SEND_ERR_TIME, freq)

if error_asserted < 0:
    print("No error state detected! FAILED\n")
else:
    print("\nTimings Obtained:")
    print("Cockpit zone did not hear from steering zone at " + str(CP_FOUND_ERROR_TIME) + "us")
    print("Cockpit zone informs drivetrain zone at " + str(CP_SEND_ERR_TIME) + "us")
    print("Drivetrain zone receives information at " + str(DT_RECV_ERR_TIME) + "us")
    print("Drivetrain zone asserts error at " + str(error_asserted) + "us")
    print("Total time taken: " + str(error_asserted - CP_FOUND_ERROR_TIME) + "us")

print("")
print("Please revive the steering zone")
print("Press enter to continue...")
x = input()

print("")
print("*****************************************************")
print("***************** Testcase 3: ***********************")
print("*****************************************************\n")
print("Checking time taken to enter error state when drivetrain zone fail")
print("Remember to revive your steering zone from last testcase!")
print("When you press enter, you have 10 seconds to kill the drivetrain zone")
print("Press enter to continue:")
x = input()

# Test for 10 second
num_samples = 1000000

# Get samples from buffer
data = dig.getSamples(1)
dig.startAcquisition(num_samples)
data = dig.getSamples(num_samples)

# Process Steering Zone Error
error_asserted = checkEdge(data, ST_ERROR, 0, freq)
ST_RECV_ERR_TIME = getPrevEdge(data, ST_RECV, error_asserted, freq)
CP_SEND_ERR_TIME = getPrevEdge(data, CP_SEND, ST_RECV_ERR_TIME, freq)
CP_FOUND_ERROR_TIME = getPrevEdge(data, CP_SEND, CP_SEND_ERR_TIME, freq)

if error_asserted < 0:
    print("No error state detected! FAILED\n")
else:
    print("\nTimings Obtained:")
    print("Cockpit zone did not hear from drivetrain zone at " + str(CP_FOUND_ERROR_TIME) + "us")
    print("Cockpit zone informs steering zone at " + str(CP_SEND_ERR_TIME) + "us")
    print("Steering zone receives information at " + str(ST_RECV_ERR_TIME) + "us")
    print("Steering zone asserts error at " + str(error_asserted) + "us")
    print("Total time taken: " + str(error_asserted - CP_FOUND_ERROR_TIME) + "us")

print("")
print("Please revive the drivetrain zone")
print("Press enter to continue...")
x = input()

print("")
print("*****************************************************")
print("***************** Testcase 4: ***********************")
print("*****************************************************\n")
print("Checking time to initialize system and send the first message")
print("Please remove power from the cockpit zone before pressing enter")
print("When you press enter, please apply power within 5 seconds")
print("Press enter to continue:")
x = input()

# Test for 10 second
num_samples = 1000000

# Get samples from buffer
data = dig.getSamples(1)
dig.startAcquisition(num_samples)
data = dig.getSamples(num_samples)

# Process Steering Zone Error
powered_on = checkEdge(data, VDD_ON, 0, freq)
first_message_sent = checkEdge(data, CP_SEND, 10, freq)


print("\nTimings Obtained:")
print("Powered on at " + str(powered_on) + "us")
print("First message sent at " + str(first_message_sent) + "us\n")

if (first_message_sent < powered_on):
    print("FAILED: Cockpit Zone sent message before power on")
    print("Was reset button held down?")
else:
    delay = first_message_sent - powered_on
    print("Delay was " + str(delay) + "us")

print("")
print("Press enter to continue...")
x = input()

print("")
print("*****************************************************")
print("***************** Testcase 5: ***********************")
print("*****************************************************\n")
print("Checking for UWB Packet Loss")
print("Please ensure that all three zones are functioning properly")
print("Press enter to continue:")
x = input()

# Test for 5 second
num_samples = 500000

# Get samples from buffer
data = dig.getSamples(1)
dig.startAcquisition(num_samples)
data = dig.getSamples(num_samples)

# Count edges
CP_SEND_COUNT = countEdges(data, CP_SEND)
DT_SEND_COUNT = countEdges(data, DT_SEND)
ST_SEND_COUNT = countEdges(data, ST_SEND)

CP_RECV_COUNT = countEdges(data, CP_RECV)
DT_RECV_COUNT = countEdges(data, DT_RECV)
ST_RECV_COUNT = countEdges(data, ST_RECV)

print("\nValues Obtained:")
print("Cockpit Zone Messages Sent: " + str(CP_SEND_COUNT))
print("Steering Zone Messages Received: " + str(ST_RECV_COUNT))
print("Drivetrain Zone Messages Received: " + str(DT_RECV_COUNT))

print("Cockpit Zone Messages Received: " + str(CP_RECV_COUNT))
print("Steering Zone Messages Sent: " + str(ST_SEND_COUNT))
print("Drivetrain Zone Messages Sent: " + str(DT_SEND_COUNT))

CP_SEND_DISPARITY = abs(CP_SEND_COUNT - ST_RECV_COUNT - DT_RECV_COUNT)
CP_RECV_DISPARITY = abs(CP_RECV_COUNT - ST_SEND_COUNT - DT_SEND_COUNT)

if (CP_SEND_DISPARITY > 2 or CP_RECV_DISPARITY > 2):
    print("\nThere seems to be some packet loss")
    pkt_loss = abs(CP_RECV_DISPARITY + CP_SEND_DISPARITY - 2) / (CP_SEND_COUNT + CP_RECV_COUNT) * 100
    print("It is estimated to be around " + str(pkt_loss) + "%")
else:
    print("\nThere seems to be no obvious packet loss in 5 seconds")

print("")
print("Press enter to continue...")
x = input()