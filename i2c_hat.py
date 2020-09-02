"""
This program was written for the Li-Polymer Hat (SW6106) to get
data from the Shield over I2C protocol. This code is for analyzing:

1. The voltage of the battery attached to the HAT.
2. The Temperature of the built-in NTC of the HAT.
3. Any hardware related issues received by the IRQ Pin.

Extra information regarding:

2. Received value of the Temperature corresponds to the ADC Value and must be checked with Datasheet for further information.
3. Hardware related issue is received as a 1 byte binary value. If any bit is 1 this indicates a specific issue depending on the bit's position.

-To the HAT:

* I2C adress: 0x3C
* Register adresses for Battery: 0x14, 0x15
* Register adresses for temperature: 0x1B, 0x1C
* Register adress for IRQ-received issues: 0x0A


September 2020

Lucas Canete
"""
import time
import smbus

bus = smbus.SMBus(1)

"""
#bus.write_byte_data(i2c_adress,register_adress,value_to_send)
#bus.read_byte_data(i2c_adress,regiser_adress)

"""
dev_adress = 0x3C
reg = {"battery":[0x14,0x15],"temperature":[0x1B,0x1C],"IRQ":0x0A}

def write(adr):

	bus.write_byte_data(dev_adress,adr,1) #one to activate the register


def read(adr):

        value = bus.read_byte_data(dev_adress,adr) 
        return value

#n = number
#p = position
#b = bit

def modifyBit( n,  p,  b):

    mask = 1 << p
    return (n & ~mask) | ((b << p) & mask)


def getBattery():

	write(reg["battery"][0]) #write to first battery register
	time.sleep(0.1)

	bat1 = read(reg["battery"][0])

	write(reg["battery"][1]) #write to second battery register
	time.sleep(0.1)

	bat2 = read(reg["battery"][1])

	#set Bits from 4 to 7 to 0 as they are not used
	for i in range(4,8):
		bat2 = modifyBit(bat2,i,0)

	value = str((((bat2<<8) | bat1)*0.0012)) # battery Value

	return value[:4]


def getTemperature():

	write(reg["temperature"][0]) #write to first register of temperature
	time.sleep(0.1)

	temp1 = read(reg["temperature"][0])

	write(reg["temperature"][1]) #wirte to second register of temperature
	time.sleep(0.1)

	temp2 = read(reg["temperature"][1])

	#set bits from 0 to 3 to 0 as they are not used
	for i in range(0,4):
		temp1 = modifyBit(temp1,i,0)

	value = ((temp1<<4) | temp2)
	return str(value)


def getError():

	write(reg["IRQ"])
	time.sleep(0.1)

	error = read(reg["IRQ"])

	value = bin(error)
	return str(error)


while True:

	battery = getBattery()
	temperature = getTemperature()
	error = getError()

	print("Battery: "+ battery +' V.' + " Temperature: " + temperature+" Error: "+ error)
