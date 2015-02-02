###################################################
# Submission	: Assignment 3 - CS39006 Spring 2015
# Date 			: 2 FEB 2015
# Submitted by 	: NEVIN VALSARAJ (12CS10032)
# 				: PRANJAL PANDEY (12CS30026)
###################################################

#!/bin/env python
import sys
from operator import itemgetter

if len(sys.argv) < 2:
	print "Parameters : data file and packetsize(defaults to 128 bytes)"
	exit
elif len(sys.argv) == 2:
	packetSize = 128
else:
	packetSize = int(sys.argv[2])

rawData = open(sys.argv[1])
data = []

print "\nStatistics : \n"

for line in rawData:
	if('udp' in line.strip().lower()):
		data.append(line.strip().split())

data[:] = [[element[0], element[1], int(element[6][22:-1]), int(element[7][27:-1])] for element in data]

groupedData_keys = [(element[2], element[3]) for element in data]
groupedData = dict.fromkeys(groupedData_keys)
packetType_keys = ['+', '-', 'r', 'delay', 'ifReceived']

for element in data:
	key = (element[2], element[3])
	if groupedData[key] is None:
		groupedData[key] = dict.fromkeys(packetType_keys)
	groupedData[key][element[0]] = element[1]

for key in groupedData:
	element = groupedData[key]
	if element['+'] is not None:
		if element['r'] is not None:
			element['ifReceived'] = True
			element['delay'] = float(element['r']) - float(element['+'])
		else:
			element['ifReceived'] = False

temp = [element for element in data if element[0] == '+']
sentPackets = sorted(temp, key = itemgetter(1))
firstTranmissionTime = float(sentPackets[0][1])

temp = [element for element in data if element[0] == 'r']
receivedPackets = sorted(temp, key = itemgetter(1))
lastReceivedTime = float(sentPackets[-1][1])

totalTime = lastReceivedTime - firstTranmissionTime

temp = [groupedData[key] for key in groupedData if groupedData[key]['ifReceived'] is not None]
totalBytesReceived = packetSize * len(temp)
throughPut = totalBytesReceived/totalTime
print "Throughput = " + str(throughPut) + " bytes/second"

delays = [groupedData[key]['delay'] for key in groupedData if groupedData[key]['ifReceived'] is not None]
totalDelay = sum(delays)
print "Total delay = " + str(totalDelay) + " seconds"
