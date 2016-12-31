#!/bin/bash
cd `dirname $0`
fname=$1

function test_linux()
{
    Inc="-I/usr/local/include/opencv -I/usr/local/include -I/usr/local/include/tesseract -I/usr/local/include/leptonica"
    Lib="-L/usr/local/lib  -lopencv_core  -lopencv_highgui -lopencv_imgproc  -lrt -lpthread -lm -ldl -L/usr/local/lib -ltesseract -ltinyxml -lpython2.7 -ljsoncpp -lzbar"
    gccx="g++ -w -o unit_exec $fname common.cpp  $Inc $Lib"
    echo $gccx
    $gccx
    ret=$?
    [ "$ret" -eq "0" ]&&echo -e "\e[1;31m编译测试成功\e[0m"||echo -e "\e[1;31m编译测试失败！！！\e[0m"
}

function test_mac()
{

    IncLib="-I/Users/yjm/Applications/anaconda/include/ -L/Users/yjm/Applications/anaconda/lib -lpython2.7 "
    for lib in "opencv3" "tesseract" "zbar" "jsoncpp";do
        IncLib="$IncLib `pkg-config --cflags $lib  pkg-config --libs $lib`" 
    done

    gccx="g++ -o unit_exec $fname common.cpp $IncLib"
    echo $gccx
    $gccx
    ret=$?
    [ "$ret" -eq "0" ]&&echo -e "\033[1;31m编译测试成功\033[0m"||echo -e "\033[1;31m编译测试失败！！！\033[0m"

}

#test_mac
test_linux

[ -f ./unit_exec ]&&echo -e "\033[1;31m运行测试:\033[0m"
./unit_exec && rm -f ./unit_exec

