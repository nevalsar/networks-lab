###################################################
# Submission	: Assignment 3 - CS39006 Spring 2015
# Date 		: 2 FEB 2015
# Submitted by 	: NEVIN VALSARAJ (12CS10032)
# 		: PRANJAL PANDEY (12CS30026)
###################################################

#!/usr/bin/env python
import sys
from operator import itemgetter
import math
import numpy as np
import matplotlib.pyplot as plt

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
	totalBitsReceived = packetSize * len(temp)
	throughPut = totalBitsReceived/totalTime
	# print "Throughput = " + str(throughPut) + " bits/second"

	delays = [groupedData[key]['delay'] for key in groupedData if groupedData[key]['ifReceived'] is True]
	totalDelay = sum(delays)/len(delays)
	# print "Total delay = " + str(totalDelay) + " seconds"
	jitter = np.std(delays)

	return (throughPut, totalDelay, jitter)

def getAverageParams(power):
	dataRate = int(math.pow(2, power))
	if dataRate > 128:
		packetSize = 32
	else:
		packetSize = dataRate / 4
	csmaValues = []
	for i in range(1, 11):
		fileName = "csma_" + str(dataRate) + "_" + str(i) + ".tr"
		print fileName
		rawData = open(fileName, 'r')
		csmaValues.append(getCSMAParams(rawData, packetSize))
	throughPuts = [element[0] for element in csmaValues]
	delays = [element[1] for element in csmaValues]
	jitters = [element[2] for element in csmaValues]
	avgDelay = float(sum(delays))/10
	avgThroughPut = float(sum(throughPuts))/10
	avgJitter = float(sum(jitters))/10

	print "Data Rate \t\t= " + str(dataRate)
	print "-----------------------------------"
	print "Avg Throughput \t\t= " + str(avgThroughPut)
	print "SD of Throughput \t= " + str(np.std(throughPuts))
	print "Avg Delay \t\t= " + str(avgDelay)
	print "SD of Delay \t\t= " + str(np.std(delays))
	print "Avg Jitter \t\t= " + str(avgJitter)
	print "SD of Jitter \t\t= " + str(np.std(jitters))
	print

	return ([avgThroughPut, np.std(throughPuts), avgDelay, np.std(delays), avgJitter, np.std(jitters)])

def plotValues(avgThroughPutArray, SDThroughPutArray, avgDelayArray, SDDelayArray, avgJitterArray, SDJitterArray):
	x = np.array([16, 32, 64, 128, 256, 512, 1024])

	# THROUGHPUT
	y = np.array(avgThroughPutArray)
	e = np.array(SDThroughPutArray)

	plt.xlabel('Data Rate (Kbps)')
	plt.ylabel('Data Rate (Kbps)')
	plt.errorbar(x, y, e, linestyle='-', fmt='o', ecolor='r')
	plt.show()

	# DELAY
	y = np.array(avgDelayArray)
	e = np.array(SDDelayArray)

	plt.xlabel('Data Rate (Kbps)')
	plt.ylabel('Data Rate (Kbps)')
	plt.errorbar(x, y, e, linestyle='-', fmt='o', ecolor='r')
	plt.show()

	# JITTER
	y = np.array(avgJitterArray)
	e = np.array(SDJitterArray)

	plt.xlabel('Data Rate (Kbps)')
	plt.ylabel('Data Rate (Kbps)')
	plt.errorbar(x, y, e, linestyle='-', fmt='o', ecolor='r')
	plt.show()


def main():
    avgThroughPutArray = []
    avgDelayArray = []
    avgJitterArray = []
    SDThroughPutArray = []
    SDDelayArray = []
    SDJitterArray = []

    for power in range(4, 11):
	value = getAverageParams(power)

	avgThroughPutArray.append(value[0])
	avgDelayArray.append(value[1])
	avgJitterArray.append(value[2])
	SDThroughPutArray.append(value[3])
	SDDelayArray.append(value[4])
	SDJitterArray.append(value[5])

    plotValues(avgThroughPutArray, avgDelayArray, avgJitterArray, SDThroughPutArray, SDDelayArray, SDJitterArray)

if __name__ == "__main__":
    main()
