#!/usr/bin/env python

from subprocess import call

filenames = ['file1.txt', 'file2.txt', 'file3.txt']
foldernames = [str(x) for x in range(8)]

call(['mkdir', 'files'])

for folder in foldernames:
    call(['mkdir', 'files/'+ folder])

for folder in foldernames:
    for file in filenames:
        call(['touch', 'files/' + folder + '/' + file])
