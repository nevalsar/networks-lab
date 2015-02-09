#!/usr/bin/env bash
runOnceNetworks(){
     ./waf --run "scratch/csma --PacketSize=$1 --Interval=$2"
     add_float="$(echo "$1/$2"|bc -q -l)"
     add=${add_float/\.*}
     mv csma_assign.tr csma_$add.tr
}

runOnceNetworks 4 0.25 1
runOnceNetworks 4 0.25 2
runOnceNetworks 4 0.25 3
runOnceNetworks 4 0.25 4
runOnceNetworks 4 0.25 5
runOnceNetworks 4 0.25 6
runOnceNetworks 4 0.25 7
runOnceNetworks 4 0.25 8
runOnceNetworks 4 0.25 9
runOnceNetworks 4 0.25 10

runOnceNetworks 8 0.25 1
runOnceNetworks 8 0.25 2
runOnceNetworks 8 0.25 3
runOnceNetworks 8 0.25 4
runOnceNetworks 8 0.25 5
runOnceNetworks 8 0.25 6
runOnceNetworks 8 0.25 7
runOnceNetworks 8 0.25 8
runOnceNetworks 8 0.25 9
runOnceNetworks 8 0.25 10

runOnceNetworks 16 0.25 1
runOnceNetworks 16 0.25 2
runOnceNetworks 16 0.25 3
runOnceNetworks 16 0.25 4
runOnceNetworks 16 0.25 5
runOnceNetworks 16 0.25 6
runOnceNetworks 16 0.25 7
runOnceNetworks 16 0.25 8
runOnceNetworks 16 0.25 9
runOnceNetworks 16 0.25 10

runOnceNetworks 32 0.25 1
runOnceNetworks 32 0.25 2
runOnceNetworks 32 0.25 3
runOnceNetworks 32 0.25 4
runOnceNetworks 32 0.25 5
runOnceNetworks 32 0.25 6
runOnceNetworks 32 0.25 7
runOnceNetworks 32 0.25 8
runOnceNetworks 32 0.25 9
runOnceNetworks 32 0.25 10

runOnceNetworks 32 0.125 1
runOnceNetworks 32 0.125 2
runOnceNetworks 32 0.125 3
runOnceNetworks 32 0.125 4
runOnceNetworks 32 0.125 5
runOnceNetworks 32 0.125 6
runOnceNetworks 32 0.125 7
runOnceNetworks 32 0.125 8
runOnceNetworks 32 0.125 9
runOnceNetworks 32 0.125 10

runOnceNetworks 32 0.0625 1
runOnceNetworks 32 0.0625 2
runOnceNetworks 32 0.0625 3
runOnceNetworks 32 0.0625 4
runOnceNetworks 32 0.0625 5
runOnceNetworks 32 0.0625 6
runOnceNetworks 32 0.0625 7
runOnceNetworks 32 0.0625 8
runOnceNetworks 32 0.0625 9
runOnceNetworks 32 0.0625 10

runOnceNetworks 32 0.03125 1
runOnceNetworks 32 0.03125 2
runOnceNetworks 32 0.03125 3
runOnceNetworks 32 0.03125 4
runOnceNetworks 32 0.03125 5
runOnceNetworks 32 0.03125 6
runOnceNetworks 32 0.03125 7
runOnceNetworks 32 0.03125 8
runOnceNetworks 32 0.03125 9
runOnceNetworks 32 0.03125 10
