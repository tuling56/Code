#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
    Fun:线上测试时：
        图像预处理和归一化(根据需要选择cv模块或者是PIL模块),
        不存在按目录进行的分类问题
    Ref:
    Date:2016/9/14
    Author:tuling56
'''
import os
import sys
import re

reload(sys)
sys.setdefaultencoding('utf-8')

HAS_CV=False
try:
    sys.path.append("cv模块所在")
    import cv2
    HAS_CV=True
except Exception,e:
    print "no cv model,use PIL instead"
    from PIL import Image

import numpy as np

curpath=os.path.split(os.path.realpath(sys.argv[0]))[0]
srcpath=os.path.join(curpath.rsplit(os.sep,3)[0],"result")
datapath=os.path.join(curpath,'data')


'''
    图像向量化并保存样本数据
'''
class Img2Vec(object):
    def __init__(self,HAS_CV,srcpath,datapath):
        self.HAS_CV=HAS_CV
        self.datapath=datapath	# 向量化后的数据
        self.srcpath=srcpath	# 源数据图像
        self.train_samples=[]   # 训练矩阵
        self.train_whats=[]      # 样本标签矩阵
        self.filelist=[]        # 训练样本
    '''
        获取所有的训练样本，并对样本进行汇总（每类有多少个）
        返回：所有的训练样本
    '''
    def processdir(self):
        peoples={}
        for root, dirs, files in os.walk(self.srcpath):
            p_filelist=[]
            mpeople=re.split('\\\\',root)[-1]
            mpeople=os.path.split(mpeople)[1]
            for file in files:
                mclass=re.split('\\\\',root)[-1]
                readfile= os.path.join(root, file)
                if 'xuanzeti_answer' not in readfile:
                    continue
                ftype=os.path.splitext(readfile)[1]
                if ftype.lower() in ['.jpg','.bmp','.jpeg','.png']:
                    p_filelist.append(readfile)
                    self.filelist.append(readfile)
                    fname = os.path.split(readfile)[1]
                    self.train_whats.append(mpeople.rsplit('.',1)[0]+"."+fname.rsplit('.',1)[0])
                #print dirs,fname
            if len(p_filelist)!=0:
                peoples[mpeople]=len(p_filelist)
        print peoples,"total:",len(self.filelist)
        print '\n'.join(self.train_whats)

    '''
        对每个训练样本进行矩阵化
        返回：训练矩阵和样本标签矩阵
    '''
    def processpic(self,picname):
        if self.HAS_CV:
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
        self.train_samples.append(norm_ndimg.flatten().tolist())


    '''
        保存训练数据
    '''
    def savetrdata(self,save=True):
	print np.array(self.train_samples)
        dd=np.array(self.train_samples)
        #是否保存样本矩阵
        np.savetxt(os.path.join(self.datapath,"samples"),np.array(self.train_samples),delimiter=',',fmt='%5.4f')  # 保留4位小数
        #np.savetxt(os.path.join(self.datapath,"whats"),np.array(self.train_whats))
        f=open(os.path.join(self.datapath,'whats'),'w')
        f.write('\n'.join(self.train_whats))
        f.close()

    '''
        Entry
    '''
    def img2vec(self,save=True):
        self.processdir()
        for file in self.filelist:
            self.processpic(file)
        if save:
            self.savetrdata()


'''
	测试（数据准备）
'''
from sklearn import cross_validation # 用于训练和测试分开
from sklearn import preprocessing    # 预处理
class LoadData(object):
    def __init__(self):
        self.samplesf='./data/samples'
        self.labelsf='./data/whats'
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
    cpp调用接口(注意路径)
'''
def img_proc(projectpath):
    #图片向量化
    srcpath=projectpath+"/result"                                   #os.path.join(projectpath,"result")
    datapath=projectpath+"/parts/ocr/online/data"                   #os.sep.join(projectpath,"parts","ocr","online","data")
    #print "[srcpathnew]:",srcpath
    #print "[datapathnew]:",datapath
    Cimgvec=Img2Vec(HAS_CV,srcpath,datapath)
    Cimgvec.img2vec()


'''
    程序入口
'''
if __name__ == "__main__":
    print "\t[curpath]:",curpath
    print '\t[srcpath]:',srcpath
    print '\t[datapath]',datapath
    
    #图片向量化（输入的单选择题图像）
    srcpath="E:\\CV\\paperocr\\trash\\paperocr_deploy_dev\\result\\img072"
    Cimgvec=Img2Vec(HAS_CV,srcpath,datapath)
    Cimgvec.img2vec()
    
    # 测试
    #Ltest=LoadData()
    #Ltest.loaddata()
