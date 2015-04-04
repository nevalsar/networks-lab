#!/usr/bin/env python

from subprocess import call

filenames = ['random', 'something', 'song', 'movie', 'woohoo']
foldernames = [str(x) for x in range(8)]

call(['mkdir', 'files'])

for folder in foldernames:
    call(['mkdir', 'files/'+ folder])

for folder in foldernames:
    for file in filenames:
        call(['touch', 'files/' + folder + '/' + folder + '_' + file])
