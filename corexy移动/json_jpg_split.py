import os
import shutil
import argparse


def separate_files(source_dir, img_dir, json_dir):
    """
    将源文件夹中的图片和JSON文件分别移动到图片文件夹和JSON文件夹

    参数:
        source_dir: 源文件夹路径
        img_dir: 图片存放文件夹路径
        json_dir: JSON文件存放文件夹路径
    """
    # 确保目标文件夹存在
    os.makedirs(img_dir, exist_ok=True)
    os.makedirs(json_dir, exist_ok=True)

    # 图片文件常见扩展名
    image_extensions = ('.jpg', '.jpeg', '.png', '.gif', '.bmp', '.tiff', '.webp')

    # 遍历源文件夹中的所有文件
    for filename in os.listdir(source_dir):
        source_path = os.path.join(source_dir, filename)

        # 只处理文件，不处理文件夹
        if os.path.isfile(source_path):
            # 检查文件扩展名
            if filename.lower().endswith(image_extensions):
                # 移动图片文件
                dest_path = os.path.join(img_dir, filename)
                shutil.move(source_path, dest_path)
                print(f"移动图片: {filename} -> {img_dir}")
            elif filename.lower().endswith('.json'):
                # 移动JSON文件
                dest_path = os.path.join(json_dir, filename)
                shutil.move(source_path, dest_path)
                print(f"移动JSON文件: {filename} -> {json_dir}")


if __name__ == "__main__":
    # 设置命令行参数
    parser = argparse.ArgumentParser(description='分离文件夹中的图片和JSON文件')
    parser.add_argument('source_dir', help='源文件夹路径')
    parser.add_argument('--img_dir', default='images', help='图片存放文件夹路径，默认是当前目录下的images')
    parser.add_argument('--json_dir', default='jsons', help='JSON文件存放文件夹路径，默认是当前目录下的jsons')

    args = parser.parse_args()

    # 执行分离操作
    separate_files(args.source_dir, args.img_dir, args.json_dir)
    print("文件分离完成!")
