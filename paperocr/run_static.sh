#!/bin/bash
cd `dirname $0`
if [ $# == 0 ];then
	compile="-c"
	test="-t"
elif [ $# == 1 ];then
	[ $1 == "-t" ]&&test="-t"
	[ $1 == "-c" ]&&compile="-c"
	[ $1 != "-t" -a $1 != "-c" ]&&echo "wrong paras,please use -t or -c para"&&exit 1
elif [ $# == 2 ];then
	[ $1 == "-t"]||[ $1 == "-c" ]&&compile="-c"
	[ $2 == "-t"]||[ $2 == "-c" ]&&test="-t"
else
	echo "wrong paras";	exit 1
fi

#### 编译(只选择依赖的库，部署使用)
if [ "$compile" == "-c" ];then
	echo -e "\e[1;31m编译中....\e[0m"
	IncLib="`pkg-config --cflags opencv` `pkg-config --cflags tesseract`  `pkg-config --libs opencv` `pkg-config --libs tesseract` `pkg-config --libs tinyxml` -lpython2.7 -ljsoncpp -lzbar"
	gccx="g++ -w -g -o main  main.cpp getanswer.cpp preciseloc.cpp roughloc.cpp common.cpp savetotrain.cpp  parts/ocr_cnn.cpp parts/ocr_tess.cpp parts/part_select.cpp parts/part_xuehao.cpp parts/part_zuguan.cpp $IncLib "
	echo $gccx
	$gccx
	echo -e "\e[1;31m编译完成\e[0m"
fi

#### 测试
if [ "$test" == "-t" ];then
	echo -e "\e[1;31m运行测试试卷:\e[0m"
	echo "./main ./data/img072.jpg"
	./main ./data/img072.jpg && rm -f ./main

	#echo -e "\e[1;31m运行结果:\e[0m"
	#ls -lh ./result/img072

	# #tesseract 测试
	#cat ./result/img072/img072_tess.txt

	# #cnn 测试
	# python ./parts/ocr/online/img_proc.py
	# python ./parts/ocr/online/cnn_ocr.py
fi

exit 0
