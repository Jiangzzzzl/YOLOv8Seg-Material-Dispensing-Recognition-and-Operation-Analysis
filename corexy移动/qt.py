#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
qt_saliao_client.py
电脑端 PyQt 客户端，接收 Jetson 推流并分析。
最终版
对应nvidia端的rtsp.py
"""

import sys
import cv2
import numpy as np
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QImage, QPixmap, QPalette, QColor
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QLabel, QAction, QDockWidget,
    QVBoxLayout, QWidget, QGroupBox, QGridLayout, QPushButton,
    QPlainTextEdit, QMessageBox
)
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

import matplotlib.pyplot as plt
import pytesseract  # OCR 识别
import re           # 正则解析数字

# pytesseract.pytesseract.tesseract_cmd = r'E:\Tesseract-OCR\tesseract.exe'

# 设置全局中文字体，常用的有 SimHei、Microsoft YaHei、STSong
plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'STSong']
plt.rcParams['axes.unicode_minus'] = False  # 解决负号显示问题

# ---------------- 配置 ----------------
RTSP_URL = "rtsp://172.20.10.7:8554/saliao_stream"
ALARM_COLOR = "red"
NORMAL_COLOR = "lightgreen"
COVERAGE_THRESHOLD =70.0
# -------------------------------------


class CoveragePlot(FigureCanvas):
    """覆盖率曲线面板"""

    def __init__(self, parent=None):
        self.fig = Figure(figsize=(4, 3), dpi=100, facecolor="none")
        super().__init__(self.fig)
        self.setParent(parent)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_title("覆盖率 (%)")
        self.ax.set_ylim(0, 100)
        self.ax.set_xlim(0, 100)
        self.ax.grid(True, linestyle="--", alpha=0.6)
        self.data = []

    def update_plot(self, value):
        self.data.append(value)
        if len(self.data) > 100:
            self.data.pop(0)
        self.ax.clear()
        self.ax.plot(self.data, color="cyan", linewidth=2)
        self.ax.set_ylim(0, 100)
        self.ax.set_xlim(0, 100)
        self.ax.set_title("覆盖率 (%)")
        self.ax.grid(True, linestyle="--", alpha=0.6)
        self.draw()


class VideoPlayer(QLabel):
    """视频显示区域，支持双击全屏"""

    def __init__(self):
        super().__init__()
        self.setAlignment(Qt.AlignCenter)
        self.setStyleSheet("background-color: black;")
        self.is_fullscreen = False

    def mouseDoubleClickEvent(self, event):
        """双击切换全屏"""
        if self.is_fullscreen:
            self.parentWidget().showNormal()
            self.is_fullscreen = False
        else:
            self.parentWidget().showFullScreen()
            self.is_fullscreen = True

class MainWindow(QMainWindow):
    """主界面，只保留视频和覆盖率曲线"""

    def __init__(self):
        super().__init__()
        self.setWindowTitle("智能撒料监控系统 - 客户端")
        self.resize(1200, 800)

        # 视频显示
        self.video_label = VideoPlayer()
        self.setCentralWidget(self.video_label)

        # 覆盖率曲线
        self.coverage_plot = CoveragePlot(self)
        dock_plot = QDockWidget("覆盖率曲线", self)
        dock_plot.setWidget(self.coverage_plot)
        self.addDockWidget(Qt.RightDockWidgetArea, dock_plot)

        # 工具栏
        toolbar = self.addToolBar("控制")
        start_action = QAction("开始拉流", self)
        stop_action = QAction("停止拉流", self)
        clear_action = QAction("清空日志", self)
        toolbar.addAction(start_action)
        toolbar.addAction(stop_action)
        toolbar.addAction(clear_action)
        start_action.triggered.connect(self.start_stream)
        stop_action.triggered.connect(self.stop_stream)
        clear_action.triggered.connect(self.clear_log)

        # 日志窗口
        self.log_widget = QPlainTextEdit()
        self.log_widget.setReadOnly(True)
        dock_log = QDockWidget("日志", self)
        dock_log.setWidget(self.log_widget)
        self.addDockWidget(Qt.BottomDockWidgetArea, dock_log)

        # 定时器更新视频
        self.cap = None
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_frame)

        # 深色主题
        self.apply_dark_theme()

    def update_frame(self):
        """刷新视频帧并分析覆盖率"""
        if not self.cap:
            return
        ret, frame = self.cap.read()
        if not ret or frame is None:
            self.log("无法获取帧")
            return

        # -----------------------------
        # 简单覆盖率计算示例
        # 假设撒料是白色或亮色，背景深色
        # -----------------------------
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        _, binary = cv2.threshold(gray, 200, 255, cv2.THRESH_BINARY)
        coverage = 100.0 * np.sum(binary > 0) / (binary.shape[0] * binary.shape[1])

        # 更新曲线
        self.coverage_plot.update_plot(coverage)

        # 转换为 Qt 图像显示
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        h, w, ch = rgb_frame.shape
        qimg = QImage(rgb_frame.data, w, h, ch * w, QImage.Format_RGB888)
        self.video_label.setPixmap(QPixmap.fromImage(qimg))

    def start_stream(self):
        """开始拉流"""
        if self.cap is None:
            self.cap = cv2.VideoCapture(RTSP_URL)
            if not self.cap.isOpened():
                self.log("无法打开 RTSP 流")
                self.cap = None
                return
        self.timer.start(30)
        self.log("开始拉流...")

    def stop_stream(self):
        """停止拉流"""
        self.timer.stop()
        if self.cap:
            self.cap.release()
            self.cap = None
        self.video_label.clear()
        self.log("停止拉流。")

    def clear_log(self):
        """清空日志"""
        self.log_widget.clear()

    def log(self, msg):
        self.log_widget.appendPlainText(msg)

    def apply_dark_theme(self):
        """应用深色主题"""
        app.setStyle("Fusion")
        palette = QPalette()
        palette.setColor(QPalette.Window, QColor(53, 53, 53))
        palette.setColor(QPalette.WindowText, Qt.white)
        palette.setColor(QPalette.Base, QColor(25, 25, 25))
        palette.setColor(QPalette.AlternateBase, QColor(53, 53, 53))
        palette.setColor(QPalette.ToolTipBase, Qt.white)
        palette.setColor(QPalette.ToolTipText, Qt.white)
        palette.setColor(QPalette.Text, Qt.white)
        palette.setColor(QPalette.Button, QColor(53, 53, 53))
        palette.setColor(QPalette.ButtonText, Qt.white)
        palette.setColor(QPalette.BrightText, Qt.red)
        palette.setColor(QPalette.Highlight, QColor(42, 130, 218))
        palette.setColor(QPalette.HighlightedText, Qt.black)
        app.setPalette(palette)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
