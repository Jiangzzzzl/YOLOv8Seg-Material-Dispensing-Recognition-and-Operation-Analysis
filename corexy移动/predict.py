from ultralytics import YOLO
import cv2

# 加载训练好的模型（权重文件在coal_seg_train/yolov8n_seg_coal/weights/best.pt）
model = YOLO('./coal_seg_train/yolov8n_seg_coal/weights/best.pt')

# 测试单张煤屑图像
img_path = './coal_ds/images/val/coal_001.jpg'
results = model.predict(
    source=img_path,
    imgsz=640,
    conf=0.3,  # 置信度阈值（过滤低置信度预测）
    iou=0.5,   # IoU阈值（用于NMS去重）
    save=True  # 保存分割结果图像
)

# 可视化分割结果（可选）
for result in results:
    seg_mask = result.masks.data  # 分割掩码（shape: N, H, W，N=煤屑数量）
    img = cv2.imread(img_path)
    for mask in seg_mask:
        mask = mask.cpu().numpy().astype('uint8') * 255  # 转为灰度图
        img = cv2.addWeighted(img, 0.7, cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR), 0.3, 0)  # 叠加掩码
    cv2.imshow('Coal Segmentation', img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()