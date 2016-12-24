#!/bin/bash
cd `dirname $0`
fname=$1

Inc="-I/usr/local/include/opencv -I/usr/local/include -I/usr/local/include/tesseract -I/usr/local/include/leptonica"
Lib="-L/usr/local/lib  -lopencv_core  -lopencv_highgui -lopencv_imgproc  -lrt -lpthread -lm -ldl -L/usr/local/lib -ltesseract -ltinyxml -lpython2.7 -ljsoncpp -lzbar"
gccx="g++ -w -o unit_exec $fname  $Inc $Lib"
echo $gccx
$gccx
echo -e "\e[1;31m单元测试编译完成\e[0m"
