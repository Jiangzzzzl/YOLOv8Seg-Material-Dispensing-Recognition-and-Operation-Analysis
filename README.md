# YOLOv8Seg-Material-Dispensing-Recognition-and-Operation-Analysis

Using NVIDIA Jetson Xavier NX as the main controller, an optimized YOLOv8-seg instance segmentation model is run to identify the coverage status of scattered materials. 
Decision instructions are executed by a CoreXY motion structure driven by an STM32 microcontroller, allowing the discharge port to dynamically adjust its path. 
At the same time, MediaMTX streaming and a QT client provide remote RTSP monitoring and real-time overflow alerts. 
An accompanying manual operation app has been developed to support manual intervention in the material spreading process and to display the CoreXY motion animation in real time.



## 目录

- [上手指南](#1)
  - [开发前的配置&硬件要求](#2)
  - [安装步骤](#3)
- [训练步骤](#4)
- [使用说明](#5)
- [各模块测试结果](#6)
- [演示视频](#7)
- [使用到的框架](#8)
- [版本控制](#9)
- [作者](#10)
- [版权说明](#11)
- [鸣谢](#12)
  
  

### <h3 id="1">上手指南</h3>

###### <h6 id="2">开发前的配置&硬件要求</h3>

1. Nvidia Xavier NX
2. python3.8
3. USB摄像头
4. STM32C8T6
5. ESP8266
6. 步进电机
7. corexy结构搭建所需材料（之后会进行补充）
8. 杜邦线若干

###### <h6 id="3">**安装步骤**</h6>

1. 英伟达端部署代码可进行参考
   [https://github.com/Jiangzzzzl/Intelligent-assistance-system-for-blind-walking-based-on-YOLOv5.git](https://github.com/Jiangzzzzl/Yolov8_TensorRT)

2. 安装依赖
   
   ```
   pip install -r requirements.txt
   ```
   
   
   
   

### <h3 id="4">训练步骤</h3>

1.数据集准备

收集项目相关图片，利用labbleme软件进行标注。

```
├─black
    ├─images
    │  ├─test
    │  └─train
    └─labels
        ├─test
        └─train
```

2.修改配置文件



3.运行 train.py





### <h3 id="5">使用说明</h3>

1.

2.

3.

4.



### <h3 id="6">各模块测试结果</h3>

#### 1.


#### 2.

#### 3.

#### 4.手机APP端


#### 5.solidworks绘制图


#### 6.最终成品状态
![image]([https://github.com/Jiangzzzzl/YOLOv8Seg-Material-Dispensing-Recognition-and-Operation-Analysis/blob/master/%E6%92%92%E6%96%99%E6%95%B4%E4%BD%93.jpg])


### <h3 id="7">演示视频</h3>



### <h3 id="8">使用到的框架</h3>

- [pytorch](https://pytorch.org/)
  
  

### <h3 id="9">版本控制</h3>

该项目使用Git进行版本管理。您可以在repository参看当前可用版本。



### <h3 id="10">作者</h3>

Jiangzzzzl

2117154720@qq.com

CSDN:柃茶柒fffffff 



### <h3 id="11">版权说明</h3>

该项目签署了MIT 授权许可，详情请参阅 [LICENSE](https://github.com/Jiangzzzzl/Intelligent-assistance-system-for-blind-walking-based-on-YOLOv5/blob/main/LICENSE)



### <h3 id="12">鸣谢</h3>

- [https://docs.ultralytics.com/zh/models/yolov8/](https://docs.ultralytics.com/zh/models/yolov8/)

