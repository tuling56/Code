#!/bin/bash
cd `dirname $0`
fname=$1

Inc="-I/usr/local/include/opencv -I/usr/local/include -I/usr/local/include/tesseract -I/usr/local/include/leptonica"
Lib="-L/usr/local/lib  -lopencv_core  -lopencv_highgui -lopencv_imgproc  -lrt -lpthread -lm -ldl -L/usr/local/lib -ltesseract -ltinyxml -lpython2.7 -ljsoncpp -lzbar"
gccx="g++ -w -o unit_exec $fname common.cpp  $Inc $Lib"
echo $gccx
$gccx
ret=$?
[ "$ret" -eq "0" ]&&echo -e "\e[1;31m编译测试成功\e[0m"||echo -e "\e[1;31m编译测试失败！！！\e[0m"

[ -f ./unit_exec ]&&echo -e "\e[1;31m运行测试:\e[0m"
./unit_exec 
