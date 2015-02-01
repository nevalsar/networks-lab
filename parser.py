###################################################
# Submission	: Assignment 3 - CS39006 Spring 2015
# Date 			: 2 FEB 2015
# Submitted by 	: NEVIN VALSARAJ (12CS10032)
# 				: PRANJAL PANDEY (12CS30026)
###################################################

#!/bin/env python
rawData = open("rawData.tr")
data = []

print "\nStatistics : \n"

for line in rawData:
	if('udp' in line.strip().lower()):
		data.append(line.strip().split())

data[:] = [[element[0], element[1], element[2][10:11], element[18]] for element in data]

# First tranmission
for i in range(len(data)):
	if '-' in data[i][0] and '0' in data[i][2]:
		print "First transmitted \t: " + data[i][1]
		break

# Last tranmission
for i in range(1, len(data) + 1) :
	if '-' in data[-i][0] and '0' in data[-i][2]:
		print "Last transmitted \t: " + data[-i][1]
		break

# First received
for i in range(len(data)):
	if 'r' in data[i][0] and '1' in data[i][2]:
		print "First received \t\t: " + data[i][1]
		break

# Last received
for i in range(1, len(data) + 1) :
	if 'r' in data[-i][0] and '1' in data[-i][2]:
		print "Last received \t\t: " + data[-i][1]
		break

# Sum of all end-to-end delays
tempData = [element for element in data if not ('r' in element[0] and int(element[2]) == 0)]
keys = [element[3] for element in tempData]
groupedData = dict.fromkeys(keys)
for i in range(len(tempData)):
	if groupedData[tempData[i][3]] is None:
		groupedData[tempData[i][3]] = []
		groupedData[tempData[i][3]].append(tempData[i][:-1])
	else:
		groupedData[tempData[i][3]].append(tempData[i][:-1])

delays = []
for key in groupedData:
	element = groupedData[key]
	sentPacket = None
	receivedPacket = None
	for packet in element:
		if '+' in packet[0]:
			sentPacket = float(packet[1])
		elif 'r' in packet[0]:
			receivedPacket = float(packet[1])
	if sentPacket is not None and receivedPacket is not None:
			delays.append(receivedPacket - sentPacket)
print "Total end-to-end delay \t: " + str(sum(delays))
