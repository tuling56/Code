#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
	Fun:cnn模型训练和测试框架
	Ref:http://www.jeyzhang.com/tensorflow-learning-notes-2.html
	Date:2016/9/19
	Author:tuling56
	功能：在原有的基础上进行模型加载和保存测试
	#!/Users/yjm/Applications/anaconda/bin/python
	#!/usr/bin/env python
'''

import os
import sys
import numpy as np
import random

reload(sys)
sys.setdefaultencoding('utf-8')

if not hasattr(sys, 'argv'):
	sys.argv  = ['']

'''
	模型加载和预测
'''
try:
	import tensorflow as tf
	from tensorflow.python.platform import gfile		# 图模型文件
except Exception,e:
	print "\033[1;31m导入模块失败\033[0m",str(e)
	sys.exit()

def cnn_predict(vsamplestr,modulename,whats):
	vsample=eval(vsamplestr) 							# 字符串形式的列表转列表
	print "\033[1;31mload model>>>\033[0m"
	prev_char=""
	with tf.Session() as sess:
		print "恢复:",modulename+'.meta'
		new_saver = tf.train.import_meta_graph(modulename+'.meta')			#恢复图模型
		new_saver.restore(sess,modulename)									#恢复数据
		# tf.get_collection() returns a list. In this example we only want the first one.
		predict=tf.get_collection('predict')[0]
		x=tf.get_collection('x')[0]
		y_=tf.get_collection('y_')[0]
		keep_prob=tf.get_collection('keep_prob')[0]

		mark=np.diag([1]*len(whats))
		
		#单项预测
		sys.argv  = ['']
		print "\033[1;31msingle predict\033[0m"
		testsample=np.array(vsample).reshape(1,len(vsample))
		prev=sess.run(predict,feed_dict={x: testsample, y_: mark, keep_prob: 1.0})
		prev_char= chr(prev.tolist().index(1)+ord(whats[0]))
		print u"[prev]:",prev_char

	print "\033[1;31mpredict done!\033[0m"

	if prev_char:
		return prev_char
	else:
		return "error"


'''
	cpp调用接口
'''
def ocr_cnn_api(vsamplestr,modulename,whats):
	#print ">>>[vsample]:",vsamplestr
	#print ">>>[module]:",modulename
	#print ">>>[whats]:",whats
	whats=list(whats)
	prev_res=cnn_predict(vsamplestr,modulename,whats)
	return prev_res


if __name__ == "__main__":
	vsamplestr=str([1]*784)
	modulename="./models/cnn/Model_ABCD"
	whats=list('ABCD')
	#ocr_cnn_api(vsamplestr,modulename,whats)
	#cnn_predict(vsamplestr,modulename,whats)
