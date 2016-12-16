# 进展说明

## 目录结构

- parts			//学号，选择，解答模块识别部分
  - models		//识别模型
    - cnn
    - tess          
- samples               //样本

### 调用层次

```mermaid
graph TD
A[main.cpp]-->B[roughloc.cpp]
B-->C[preciseloc.cpp]
C-->D[getanswer.cpp]

D-->E[part_select.cpp]
D-->F[part_xuehao.cpp]
D-->G[part_zuguan.cpp]

E-->H[ocr_cnn.cpp]
G-->H

E-->K[ocr_tess.cpp]
G-->K
K-->eng.traindata

H-->T[cnn_tensor_class.py]
T-->Model_ABCD
T-->Model_0-9

F-->zbar_xuehao_scan.cpp

```



## 事件记录

- 2016年12月14日

  > 注意在文件加入vs工程的时候将将文件的格式进行转换，不然会提示很多莫名其妙的错误
  >
  > ​
