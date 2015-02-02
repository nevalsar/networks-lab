###################################################
# Submission	: Assignment 3 - CS39006 Spring 2015
# Date 			: 2 FEB 2015
# Submitted by 	: NEVIN VALSARAJ (12CS10032)
# 				: PRANJAL PANDEY (12CS30026)
###################################################

#!/bin/env python
import sys
from operator import itemgetter
import math
import numpy as np

def getCSMAParams(rawData, packetSize):
	data = []
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
	if len(sentPackets) < 1:
		print "No sent"
		sys.exit()
	firstTranmissionTime = float(sentPackets[0][1])

	temp = [element for element in data if element[0] == 'r']
	receivedPackets = sorted(temp, key = itemgetter(1))
	lastReceivedTime = float(sentPackets[-1][1])

	totalTime = lastReceivedTime - firstTranmissionTime

	temp = [groupedData[key] for key in groupedData if groupedData[key]['ifReceived'] is not None]
	totalBytesReceived = packetSize * len(temp)
	throughPut = totalBytesReceived/totalTime
	# print "Throughput = " + str(throughPut) + " bytes/second"

	delays = [groupedData[key]['delay'] for key in groupedData if groupedData[key]['ifReceived'] is not None]
	totalDelay = sum(delays)
	# print "Total delay = " + str(totalDelay) + " seconds"

	return (throughPut, totalDelay)

for power in range(4, 11):
	dataRate = int(math.pow(2, power))
	packetSize = dataRate / 4
	csmaValues = []
	for i in range(1, 11):
		fileName = "csma_" + str(dataRate) + "_" + str(i) + ".tr"
		rawData = open(fileName, 'r')
		print dataRate, i
		csmaValues.append(getCSMAParams(rawData, packetSize))
	throughPuts = [element[0] for element in csmaValues]
	delays = [element[1] for element in csmaValues]
	avgDelay = float(sum(delays))/10
	avgThroughPut = float(sum(throughPuts))/10
	jitter = np.std(delays)

	print "Data Rate \t= " + str(dataRate)
	print "Avg Throughput \t= " + str(avgThroughPut)
	print "Avg Delay \t= " + str(avgDelay)
	print "Jitter \t\t= " + str(jitter)
	print
