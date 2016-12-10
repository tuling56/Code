#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
    Fun:制作训练样
        图像预处理和归一化（只使用了CV模块）
        按输入图像的目录进行分类
    Ref:
    Date:2016/9/14
    Author:tuling56
'''
import os
import sys
import re

reload(sys)
sys.setdefaultencoding('utf-8')

import cv2
import numpy as np
import pandas as pd

'''
    图像向量化并保存样本数据
'''
class Img2Vec(object):
    def __init__(self,datapath,whatlist,wheretosave):
        self.datapath=datapath
        self.whatlist=whatlist
        self.wheretosave=wheretosave
        self.train_samples=[]  # 训练矩阵
        self.train_labels=[]   # 样本标签矩阵
        self.filelist=[]       # 训练样本

    #获取所有的训练样本，并对样本进行汇总（每类有多少个）  返回：所有的训练样本
    def processdir(self):
        classes={}
        for root, dirs, files in os.walk(self.datapath):
            for file in files:
                mclass=re.split('\\\\',root)[-1]
                readfile= os.path.join(root, file)
                self.filelist.append(readfile)
                fname = os.path.split(readfile)[1]
                # print dirs,fname
            mclass=re.split('\\\\',root)[-1]
            if len(files)!=0:
                classes[mclass]=len(files)
        print classes,"total:",len(self.filelist)

    # 对每个训练样本进行矩阵化;返回：训练矩阵和样本标签矩阵
    def processpic(self,picname):
        img=cv2.imread(picname,0)
        rzimg=cv2.resize(img,(28,28))
        #cv2.imshow("X",img)
        #cv2.waitKey()
        ndimg=np.array(rzimg)
        fndimg=ndimg.astype(float)
        norm_ndimg=(fndimg-fndimg.min())/(fndimg.max()-fndimg.min())  # 归一化
        fname=os.path.split(picname)[1]
        label=fname[0].upper()
        if label not in self.whatlist:
            print "[wrong label]:",label
            return 0
        else:
            labelv=[0]*len(self.whatlist)
            self.whatlist.sort()
            labelv[ord(label)-ord(self.whatlist[0])]=1
        self.train_samples.append(norm_ndimg.flatten())
        self.train_labels.append(labelv)

    # 保存训练数据
    def savetrdata(self):
        #是否保存样本矩阵
        np.savetxt(os.path.join(self.wheretosave,'samples'),np.array(self.train_samples),delimiter=',',fmt='%5.4f')  # 保留4位小数
        # 标签字母保存
        '''
        lables=pd.DataFrame(self.train_labels)
        #print train_labels,lables
        lables.to_csv('data/lables',index=False,header=False)
        '''
        # 标签0-1矩阵保存
        np.savetxt(os.path.join(self.wheretosave,'labels'),np.array(self.train_labels),delimiter=',',fmt='%d')

    # Entry
    def img2vec(self,save=True):
        self.processdir()
        for file in self.filelist:
            self.processpic(file)
        if save:
            if not os.path.exists(self.wheretosave):
                os.mkdir(self.wheretosave)
            self.savetrdata()


'''
	数据准备
'''
from sklearn import cross_validation # 用于训练和测试分开
from sklearn import preprocessing    # 预处理
class LoadData(object):
    def __init__(self,samplesf,labelsf):
        self.samplesf=samplesf
        self.labelsf=labelsf
    def loaddata(self,test_ratio=0.4):
        X=np.loadtxt(self.samplesf,dtype=np.float32,delimiter=',')
        # data normalization
        norm_X=preprocessing.normalize(X)
        stand_X=preprocessing.scale(X)
        y=np.loadtxt(self.labelsf, dtype=np.float32,delimiter=',') #converters={ 0 : lambda ch : ord(ch)-ord('A')})
        #随机抽取生成训练集和测试集，其中训练集的比例为60%，测试集40%
        train_samples,train_labels,test_samples,test_labels = cross_validation.train_test_split(X, y, test_size=test_ratio, random_state=0)
        return  train_samples,train_labels,test_samples,test_labels


'''
    程序入口
'''
if __name__ == "__main__":
    #图片向量化
    srcpath_ABCD="E:\\CV\\paperocr\\data\\train_ABCD"
    dstpath_ABCD="./vecdata/abcd"

    srcpath_Num="E:\\CV\\paperocr\\data\\train_num_2530"
    dstpath_Num="./vecdata/num_own"

    Cimgvec=Img2Vec(srcpath_Num,list('0123456789'),dstpath_Num)
    Cimgvec.img2vec()

    # 测试
    #samplesf=os.path.join(dstpath_Num,'samples')
    #labelsf=os.path.join(dstpath_Num,'labels')
    #Ltest=LoadData(samplesf,labelsf)
    #Ltest.loaddata()
