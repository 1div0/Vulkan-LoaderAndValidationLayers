#!/bin/bash
#set -x
if [ -t 1 ] ; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    NC='\033[0m' # No Color
else
    RED=''
    GREEN=''
    NC=''
fi

printf "$GREEN[ RUN      ]$NC $0\n"

D=`dirname \`pwd\``
GDIR=../../../Glave/`basename $D`/

# Create a temp directory to run the test in
rm -rf vktracereplay_tmp
mkdir vktracereplay_tmp
cd vktracereplay_tmp
cp ../$GDIR/glvreplay .
cp ../$GDIR/glvtrace .
cp ../$GDIR/libglvreplay_vk.so .
cp ../$GDIR/libglvtrace_vk.so .
cp ../../demos/cube .
cp ../../demos/*png .
cp ../../demos/cube*spv .
export LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
(
    ./glvtrace   -s 1 -p cube  -o c01.glv -l0 libglvtrace_vk.so &
    P=$!
    sleep 3
    kill $P
) >/dev/null 2>&1
mv 1.ppm 1_trace.ppm
./glvreplay   -s 1 -t c01.glv >/dev/null 2>&1
#cp cube 1.ppm  # For testing this script -- force a failure
#rm 1.ppm       # For testing this script -- force a failure
cmp -s 1.ppm 1_trace.ppm
RES=$?
cd ..
rm -rf vktracereplay_tmp

if [ $RES -eq 0 ] ; then
   printf "$GREEN[  PASSED  ]$NC 1 test\n"
   exit 0
else
   printf "$RED[  FAILED  ]$NC screenshot file compare failed\n"
   printf "$RED[  FAILED  ]$NC 1 test\n"
   printf "1 TEST FAILED\n"
   exit 1
fi
