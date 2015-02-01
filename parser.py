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
packet_keys = ['+', 'r', '-']
groupedData = dict.fromkeys(keys)
for element in tempData:
	key = element[3]
	if groupedData[key] is None:
		groupedData[key] = dict.fromkeys(packet_keys)
	if '+' in element[0]:
		groupedData[key]['+'] = element[1:-1]
	elif 'r' in element[0]:
		groupedData[key]['r'] = element[1:-1]
	elif '-' in element[0]:
		groupedData[key]['-'] = element[1:-1]

delays = []

for key in groupedData:
	element = groupedData[key]
	if element['+'] is not None and element['r'] is not None:
			delays.append(float(element['r'][0]) - float(element['+'][0]))
print "Total end-to-end delay \t: " + str(sum(delays))
