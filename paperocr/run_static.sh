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
	#gccx="g++ -w -g -o main  main.cpp getanswer.cpp preciseloc.cpp roughloc.cpp  parts/ocr/ocr_cnn.cpp parts/ocr/ocr_tess.cpp parts/select.cpp tools/saveresult.cpp  `pkg-config --cflags opencv` `pkg-config --cflags tesseract`  `pkg-config --libs opencv` `pkg-config --libs tesseract` `pkg-config --libs tinyxml` -lpython2.7"
	gccx="g++ -w -o main main.cpp getanswer.cpp preciseloc.cpp roughloc.cpp parts/ocr/ocr_cnn.cpp parts/ocr/ocr_tess.cpp parts/select.cpp tools/saveresult.cpp -I/usr/local/include/opencv -I/usr/local/include -I/usr/local/include/tesseract -I/usr/local/include/leptonica -L/usr/local/lib  -lopencv_core  -lopencv_highgui -lopencv_imgproc  -lrt -lpthread -lm -ldl -L/usr/local/lib -ltesseract -ltinyxml -lpython2.7"
	echo $gccx
	$gccx
	echo -e "\e[1;31m编译完成\e[0m"
fi

#### 测试
if [ "$test" == "-t" ];then
	echo -e "\e[1;31m运行测试试卷:\e[0m"
	echo "./main ./data/img072.jpg"
	./main ./data/img072.jpg

	#echo -e "\e[1;31m运行结果:\e[0m"
	#ls -lh ./result/img072 
	
	# #tesseract 测试 	
	#cat ./result/img072/img072_tess.txt
	
	# #cnn 测试
	# python ./parts/ocr/online/img_proc.py
	# python ./parts/ocr/online/cnn_ocr.py
fi

exit 0
