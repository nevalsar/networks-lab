###################################################
# Submission	: Assignment 3 - CS39006 Spring 2015
# Date 			: 2 FEB 2015
# Submitted by 	: NEVIN VALSARAJ (12CS10032)
# 				: PRANJAL PANDEY (12CS30026)
###################################################

#!/bin/env python
rawData = open("rawData.tr")
data = []

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
		print "First received \t: " + data[i][1]
		break

# Last received
for i in range(1, len(data) + 1) :
	if 'r' in data[-i][0] and '1' in data[-i][2]:
		print "Last received \t: " + data[-i][1]
		break
