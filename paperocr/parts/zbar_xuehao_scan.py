#!/usr/bin/env python
# -*- coding:utf-8 -*-
import zbar
from PIL import Image
# 创建图片扫描对象
scanner = zbar.ImageScanner()
# 设置对象属性 
scanner.parse_config('enable')
# 打开含有二维码的图片
img = Image.open('qr.bmp').convert('L')
#获取图片的尺寸
width, height = img.size
raw = img.tostring() 
#建立zbar图片对象并扫描转换为字节信息
qrCode = zbar.Image(width, height, 'Y800', raw)
scanner.scan(qrCode)
data = '' 
for s in qrCode: data += s.data
# 删除图片对象
del img
# 输出解码结果
print data
