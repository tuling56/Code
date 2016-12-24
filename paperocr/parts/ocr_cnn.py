#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
	Fun:cnn模型训练和测试框架
	Ref:http://www.jeyzhang.com/tensorflow-learning-notes-2.html
	Date:2016/9/19
	Author:tuling56
	功能：在原有的基础上进行模型加载和保存测试
'''
import os
import sys
import re
import numpy as np
import random

reload(sys)
sys.setdefaultencoding('utf-8')


'''
	若给的是图片路径，先加载并向量化,//这部分要修改成直接传递的是图像数据
'''
HAS_CV=False
try:
    sys.path.append("cv模块所在")
    import cv2
    HAS_CV=True
except Exception,e:
    print "no cv model,use PIL instead"
    from PIL import Image

# 这部分可以放在c++程序中处理，从而避开文件的保存环节（已放在c++中程序中完成）
def processpic(picname):
    if HAS_CV:
        img=cv2.imread(picname,0)
        rzimg=cv2.resize(img,(28,28))
        #cv2.imshow("X",img)
        #cv2.waitKey()
        ndimg=np.array(rzimg)
        fndimg=ndimg.astype(float)
    else:
        im=Image.open(picname)
        img=im.convert('L')
        #print(img.format,img.size,img.mode)
        img.thumbnail((28,28))
        imgbn=np.asarray(img,dtype=np.float)
        fndimg=imgbn.T

    norm_ndimg=(fndimg-fndimg.min())/(fndimg.max()-fndimg.min())  # 归一化
    return norm_ndimg.flatten().tolist()                          # numpy转化成列表


'''
	模型训练和测试
'''
import tensorflow as tf
from tensorflow.python.platform import gfile # 图模型文件

def cnn_predict(vsamplestr,modulename,whats):
	vsample=eval(vsamplestr) 				# 字符串形式的列表转列表
	print "\033[1;31mload model>>>\033[0m"
	prev_char=""
	with tf.Session() as sess:
		new_saver = tf.train.import_meta_graph(modulemname+'.meta')			#恢复图模型
		new_saver.restore(sess,modulename )									#恢复数据
		# tf.get_collection() returns a list. In this example we only want the first one.
		predict=tf.get_collection('predict')[0]
		x=tf.get_collection('x')[0]
		y_=tf.get_collection('y_')[0]
		keep_prob=tf.get_collection('keep_prob')[0]
				
		mark=np.diag([1]*len(whats))
		
		#单项预测
		print "\033[1;31msingle predict\033[0m"
		test=np.array(vsample)
		prev=sess.run(predict,feed_dict={x: test, y_: mark, keep_prob: 1.0})
		prev_char= chr(prev.tolist().index(1)+ord(whats[0]))
		print u"[prev]:",prev_char

	print "\033[1;31mpredict done!\033[0m"

	if prev_char:
		return prev_char;
	else
		return "error"



'''
	cpp调用接口
'''
def cnn_ocr(pic,modulename,whats):
	vsample=processpic(pic)
	whats=list(whats)
	prev_res=cnn_predict(vsample,modulename,whats)
	return prev_res;


if __name__ == "__main__":
	# 图像向量化
	pic=""
	vsample=processpic(pic)

	# CNN识别
	modulename="./models/model_0-9"
	whats=list('0123456789')
	cnn_predict(vsample,modulename,whats)
