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
	训练数据加载
'''
from sklearn import cross_validation		# 用于训练和测试分开
from sklearn import preprocessing			# 预处理
from sklearn import metrics					# 用于结果判定

class LOADATA(object):
	def __init__(self):
		self.ratio=0.4	#其中训练集的比例为60%，测试集40%
		
	def loaddata(self,fsamples,flabels):
		try:
			X=np.loadtxt(fsamples,dtype=np.float32,delimiter=',').astype(float)
			# data normalization
			norm_X=preprocessing.normalize(X)
			stand_X=preprocessing.scale(X)
			y=np.loadtxt(flabels, dtype=np.float32,delimiter=',')  #converters={ 0 : lambda ch : ord(ch)-ord('A')})
		except Exception,e:
			print "加载数据失败:",str(e)
			sys.exit(0)
		
		#随机抽取生成训练集和测试集，其中训练集的比例为60%，测试集40%
		train_samples,test_samples,train_labels,test_labels = cross_validation.train_test_split(X, y, test_size=self.ratio, random_state=0)
		return	train_samples,test_samples,train_labels,test_labels

'''
	模型训练和测试
'''
import tensorflow as tf
from tensorflow.python.platform import gfile # 图模型文件

class CCNNM(object):
	def __init__(self,mname,whats):
		self.mname=mname
		self.whats=whats
		self.classnum=len(whats)

	def weight_varible(self,shape):
		initial = tf.truncated_normal(shape, stddev=0.1)
		return tf.Variable(initial)

	def bias_variable(self,shape):
		initial = tf.constant(0.1, shape=shape)
		return tf.Variable(initial)

	def conv2d(self,x, W):
		return tf.nn.conv2d(x, W, strides=[1, 1, 1, 1], padding='SAME')

	def max_pool_2x2(self,x):
		return tf.nn.max_pool(x, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='SAME')


	# 模型训练
	def train_model(self,train_samples,test_samples,train_labels,test_labels):
		sess = tf.InteractiveSession()

		# paras
		W_conv1 = self.weight_varible([5, 5, 1, 32])
		b_conv1 = self.bias_variable([32])

		# conv layer-1
		x = tf.placeholder(tf.float32, [None, 784])
		x_image = tf.reshape(x, [-1, 28, 28, 1])

		h_conv1 = tf.nn.relu(self.conv2d(x_image, W_conv1) + b_conv1)
		h_pool1 = self.max_pool_2x2(h_conv1)

		# conv layer-2
		W_conv2 = self.weight_varible([5, 5, 32, 64])
		b_conv2 = self.bias_variable([64])

		h_conv2 = tf.nn.relu(self.conv2d(h_pool1, W_conv2) + b_conv2)
		h_pool2 = self.max_pool_2x2(h_conv2)

		# full connection
		W_fc1 = self.weight_varible([7 * 7 * 64, 1024])
		b_fc1 = self.bias_variable([1024])

		h_pool2_flat = tf.reshape(h_pool2, [-1, 7 * 7 * 64])
		h_fc1 = tf.nn.relu(tf.matmul(h_pool2_flat, W_fc1) + b_fc1)

		# dropout
		keep_prob = tf.placeholder(tf.float32)
		h_fc1_drop = tf.nn.dropout(h_fc1, keep_prob)

		# output layer: softmax(注意调节这里的10:代表分类变量的个数)
		W_fc2 = self.weight_varible([1024,self.classnum])
		b_fc2 = self.bias_variable([self.classnum])

		y_conv = tf.nn.softmax(tf.matmul(h_fc1_drop, W_fc2) + b_fc2)
		y_ = tf.placeholder(tf.float32, [None, self.classnum])

		# model training
		cross_entropy = -tf.reduce_sum(y_ * tf.log(y_conv))
		train_step = tf.train.AdamOptimizer(1e-4).minimize(cross_entropy)

		correct_prediction = tf.equal(tf.arg_max(y_conv, 1), tf.arg_max(y_, 1),name="prediction")
		accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

		sess.run(tf.initialize_all_variables())

		train_num=train_samples.shape[0]
		test_num=test_samples.shape[0]

		#训练模型保存
		saver = tf.train.Saver()
		tf.add_to_collection('train_op', train_step)
			
		if not os.path.exists('./models/train_step'):
			os.makedirs('./models/train_step')

		for i in range(10000):
			rdnum=random.sample(np.arange(train_num),50)
			train_samples_batch=train_samples[rdnum,]
			train_labels_batch=train_labels[rdnum,]
			if i % 200 == 0:  #100次的时候输出一次测试精度
				train_accuacy = accuracy.eval(feed_dict={x: train_samples_batch, y_: train_labels_batch, keep_prob: 1.0})
				print("step %d, training accuracy %g"%(i, train_accuacy))
				saver.save(sess, './models/train_step/train-step-model', global_step=i)  #模型保存
			train_step.run(feed_dict = {x: train_samples_batch, y_: train_labels_batch, keep_prob: 0.5})  #训练的时候设置为0.5


		# 摘要信息保存
		#os.system("rm -rf /tmp/save_graph_logs")
		#merged = tf.merge_all_summaries()
		#train_writer = tf.train.SummaryWriter('/tmp/save_graph_logs',sess.graph) # 摘要信息保存

		#模型保存（ok）
		# 方法1：只保存图模型
		#tf.train.export_meta_graph(filename='./models/export_meta_model.meta') #模型导出

		# 方法2：只保存图模型
		#os.system("rm -rf /tmp/load")
		#tf.train.write_graph(sess.graph_def, '/tmp/load','write_graph_model.meta', False) # 计算图模型

		# 方法3：保存图模型和数据
		#saver = tf.train.Saver(tf.all_variables()) #保存索引变量的数据
		tf.add_to_collection('predict', correct_prediction)
		tf.add_to_collection('x', x)
		tf.add_to_collection('y_', y_)
		tf.add_to_collection('keep_prob',keep_prob)
		saver.save(sess,self.mname)


		# ####模型评估
		print("test accuracy %g"%(accuracy.eval(feed_dict={x: test_samples, y_: test_labels, keep_prob: 1.0}))) #测试的时候实则为1.0

		mark=np.diag([1]*self.classnum)
		
		# 单值预测
		'''
		test=test_samples[1,].reshape(1,784)
		prev=sess.run(correct_prediction,feed_dict={x: test, y_: mark, keep_prob: 1.0})
		print u"[prev]:",chr(prev.tolist().index(1)+65)
		print u"[true]:",chr(test_labels[1,].tolist().index(1)+65)
		'''

		# 混淆矩阵测试
		print "\033[1;31mbatch matrix\033[0m"
		pre_labels=[]
		swhats=sorted(self.whats)
		try:
			for sample in test_samples.tolist():
				sample=np.array(sample)
				pre_label=sess.run(correct_prediction,feed_dict={x: sample.reshape(1,784), y_: mark, keep_prob: 1.0})
				pre_labels.append(pre_label)
			pre_char_labels=[ chr(l.tolist().index(True)+ord(swhats[0])) for l in pre_labels ]		   # 预测值按真假分
			test_char_labels=[ chr(l.tolist().index(1)+ord(swhats[0])) for l in test_labels ]		   # 实际这是01分
			cmatrix=metrics.confusion_matrix(pre_char_labels, test_char_labels)
		except Exception,e:
			print "\033[1;31m异常1]:%s\033[0m" %(str(e))

		# 在混淆矩阵的基础上统计每个字符的指标
		cmatrix.dtype=np.float
		right=np.diag(cmatrix)
		true_all=cmatrix.sum(1)
		pre_all=cmatrix.sum(0)
				
		print "混淆矩阵:",cmatrix
		print "准确率:",right/pre_all
		print "召回率:",right/true_all
		print "\033[1;31mtrain over!\033[0m"

	# 模型测试1
	def test_model(self):
		print "\033[1;31mbegin load model>>>\033[0m"
		with tf.Session() as sess:
			new_saver = tf.train.import_meta_graph(self.mname+'.meta')			#恢复图模型
			new_saver.restore(sess,self.mname )						#恢复数据
			# tf.get_collection() returns a list. In this example we only want the first one.
			predict=tf.get_collection('predict')[0]
			x=tf.get_collection('x')[0]
			y_=tf.get_collection('y_')[0]
			keep_prob=tf.get_collection('keep_prob')[0]
					
			mark=np.diag([1]*self.classnum)
			
			#单项预测
			'''
			print "\033[1;31msingle predict\033[0m"
			test=test_samples[1,].reshape(1,784)
			prev=sess.run(predict,feed_dict={x: test, y_: mark, keep_prob: 1.0})
			print u"[prev]:",chr(prev.tolist().index(1)+65)
			'''

			# 混淆矩阵测试
			print "\033[1;31mbatch matrix\033[0m"
			pre_labels=[]
			swhats=sorted(self.whats)
			for sample in test_samples.tolist():
				sample=np.array(sample)
				pre_label=sess.run(predict,feed_dict={x: sample.reshape(1,784), y_: mark, keep_prob: 1.0})
				pre_labels.append(pre_label)
			pre_char_labels=[ chr(l.tolist().index(True)+ord(swhats[0])) for l in pre_labels ]			   # 预测值按真假分
			test_char_labels=[ chr(l.tolist().index(1)+ord(swhats[0])) for l in test_labels ]		   # 实际这是01分
			cmatrix = metrics.confusion_matrix(pre_char_labels, test_char_labels)
			print cmatrix

			# 在混淆矩阵的基础上统计每个字符的指标
			cmatrix.dtype=np.float
			right=np.diag(cmatrix)
			true_all=cmatrix.sum(1)
			pre_all=cmatrix.sum(0)
			
			print "准确率:",right/pre_all
			print "召回率:",right/true_all
		print "\033[1;31mpredict done!\033[0m"

	# 模型测试2(开发中)
	def testmodel2():
		with tf.Session() as persisted_sess:
			print("load graph") #加载计算图
			with gfile.FastGFile("write_graph_model.meta",'rb') as f:
				graph_def = tf.GraphDef()
				graph_def.ParseFromString(f.read())
				persisted_sess.graph.as_default()
				tf.import_graph_def(graph_def, name='') #加载图定义

				print("map variables")
				predict = persisted_sess.graph.get_tensor_by_name("prediction:0") #获取这个tensor
				tf.add_to_collection(tf.GraphKeys.VARIABLES,predict)  #将这个tensor加入到要恢复的变量中
				try:
					saver = tf.train.Saver(tf.all_variables()) # 'Saver' misnomer! Better: Persister!  #将变量恢复
				except:
					pass

				# 恢复数据
				print("load data")
				saver.restore(persisted_sess, "saver_checkpoint")  # 将变量的数据重新加载到各个tensor

				#重现运算
				test=test_samples[1,].reshape(1,784)
				mark=np.diag([1]*self.classnum)
				prev=persisted_sess.run(predict,feed_dict={x: test, y_: mark, keep_prob: 1.0})
				swhats=sorted(self.whats)
				print u"[prev]:",chr(prev.tolist().index(1)+ord(swhats[0]))
				print u"[true]:",chr(test_labels[1,].tolist().index(1)+ord(swhats[0]))


if __name__ == "__main__":
	# 数据准备
	fsamples='./data/num/samples'
	flabels='./data/num/labels'
	mdata=LOADATA()
	train_samples,test_samples,train_labels,test_labels=mdata.loaddata(fsamples,flabels)
	
	# 模型训练
	fmname='./models/digit_module'
	mcnn=CCNNM(fmname,list('0123456789'))
	mcnn.train_model(train_samples,test_samples,train_labels,test_labels)
	mcnn.test_model()
