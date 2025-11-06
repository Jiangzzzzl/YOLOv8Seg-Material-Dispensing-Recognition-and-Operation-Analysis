import threading
import subprocess
import time
import sys
import os
from queue import Queue, Empty
import logging

import cv2
import numpy as np
from ultralytics import YOLO
import serial
from PIL import Image, ImageDraw, ImageFont

# ---------- CONFIGURATION ----------
CAMERA_DEVICE = 0                      # 0 or '/dev/video0' or gstreamer pipeline
WIDTH, HEIGHT, FPS = 640, 480, 10
STREAM_NAME = "saliao_stream"
RTSP_SERVER_URL = f"rtsp://localhost:8554/{STREAM_NAME}"
MODEL_PATH = "black.engine"            # change to your model file
CONF_THRESHOLD = 0.5
IMGSZ = 512
TASK = "segment"

# 【新增】手动指定的两个坐标（需根据实际需求调整，格式：(x, y)）
MANUAL_POINT_1 = (200, 200)  # 第一个待发送的指定坐标
MANUAL_POINT_2 = (300, 250)  # 第二个待发送的指定坐标
MANUAL_POINT_3 = (300, 250)
MANUAL_POINT_4 = (310, 280)
MANUAL_POINT_5 = (320, 300)
MANUAL_POINT_6 = (330, 300)
MANUAL_POINT_7 = (350, 350)
MANUAL_POINT_8 = (380, 350)
MANUAL_POINT_9 = (380, 360)
MANUAL_POINT_10 = (400, 380)


FFMPEG_CMD = [
    'ffmpeg', '-y', '-an', '-f', 'rawvideo', '-pix_fmt', 'bgr24',
    '-s', f'{WIDTH}x{HEIGHT}', '-r', str(FPS), '-i', '-',
    '-c:v', 'libx264', '-preset', 'ultrafast', '-tune', 'zerolatency',
    '-g', '10', '-b:v', '500k', '-maxrate', '700k', '-bufsize', '1000k',
    '-f', 'rtsp', RTSP_SERVER_URL
]

SERIAL_PORT = "/dev/ttyTHS0"
SERIAL_BAUD = 115200
SERIAL_TIMEOUT = 0.5

# feed area (x1,y1,x2,y2)
FEED_AREA_RECT = [100, 100, 400, 400]
END_THRESHOLD = 50.0    # >=95% -> send 'end' over serial and mark complete

# alarm pixel (y,x) used to encode a simple signal in the stream (optional)
ALARM_SIGNAL_POS = (0, 0)

# 【新增】串口发送状态：1=待发第一个手动坐标，2=待发第二个手动坐标，11=自动路径规划
SERIAL_STATE = 1  # 初始状态：先执行第一个手动坐标发送
# -----------------------------------

logging.basicConfig(level=logging.INFO, format='[%(asctime)s] %(levelname)s: %(message)s')

stop_event = threading.Event()
frame_q = Queue(maxsize=2)      # raw frames for inference
out_q = Queue(maxsize=2)        # processed frames for ffmpeg push (BGR)

latest_pos = None
latest_coverage = 0.0
pos_lock = threading.Lock()


def get_chinese_font():
    font_paths = [
        "msyh.ttc",
        "C:\\Windows\\Fonts\\msyh.ttc",
        "/usr/share/fonts/truetype/msttcorefonts/msyh.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/System/Library/Fonts/PingFang.ttc"
    ]
    for p in font_paths:
        if os.path.exists(p):
            return p
    return None

FONT_PATH = get_chinese_font()


def draw_chinese_text_on_frame(frame, text, pos, font_size=24, color=(255,255,255)):
    try:
        img_pil = Image.fromarray(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
        draw = ImageDraw.Draw(img_pil)
        try:
            font = ImageFont.truetype(FONT_PATH, font_size) if FONT_PATH else ImageFont.load_default()
        except Exception:
            font = ImageFont.load_default()
        draw.text(pos, text, font=font, fill=(color[2], color[1], color[0]))
        return cv2.cvtColor(np.array(img_pil), cv2.COLOR_RGB2BGR)
    except Exception as e:
        logging.debug(f"draw text failed: {e}")
        return frame


def build_full_mask_from_results(results, w, h):
    full_mask = np.zeros((h, w), dtype=np.uint8)
    if not results:
        return full_mask
    try:
        res0 = results[0]
        masks = getattr(res0, 'masks', None)
        if masks is None:
            return full_mask
        # masks.data could be on CPU or GPU; use .cpu().numpy() safely
        mask_arr = masks.data.cpu().numpy()
        for m in mask_arr:
            mask_r = cv2.resize(m, (w, h), interpolation=cv2.INTER_NEAREST)
            full_mask[mask_r > 0.5] = 255
    except Exception as e:
        logging.debug(f"build mask failed: {e}")
    return full_mask


def calculate_coverage_and_outside(full_mask, feed_rect):
    h, w = full_mask.shape[:2]
    x0, y0, x1, y1 = feed_rect
    x0, y0 = max(0, x0), max(0, y0)
    x1, y1 = min(w, x1), min(h, y1)
    if x0 >= x1 or y0 >= y1:
        return 0.0, False
    inner = full_mask[y0:y1, x0:x1]
    total = inner.size
    if total == 0:
        coverage = 0.0
    else:
        covered = np.sum(inner == 255)
        coverage = (covered / total) * 100.0
    outside = np.sum((full_mask == 255)) - np.sum(inner == 255)
    has_outside = outside > 0
    return coverage, has_outside


def suggest_next_point(full_mask, feed_rect, grid_size=3):
    """选择下一个撒料点：在覆盖率最低的网格内取空白质心"""
    x0, y0, x1, y1 = feed_rect
    x0, y0 = max(0, x0), max(0, y0)
    x1, y1 = min(full_mask.shape[1], x1), min(full_mask.shape[0], y1)
    if x0 >= x1 or y0 >= y1:
        return None

    # 分区尺寸
    w = (x1 - x0) // grid_size
    h = (y1 - y0) // grid_size

    best_cell = None
    min_cov = 1e9
    best_pts = None

    for i in range(grid_size):
        for j in range(grid_size):
            cx0 = x0 + i * w
            cy0 = y0 + j * h
            cx1 = x0 + (i+1) * w if i < grid_size-1 else x1
            cy1 = y0 + (j+1) * h if j < grid_size-1 else y1

            cell = full_mask[cy0:cy1, cx0:cx1]
            if cell.size == 0:
                continue

            covered = np.sum(cell == 255)
            total = cell.size
            cov = covered / total if total > 0 else 0

            if cov < min_cov:
                empty = (cell == 0).astype(np.uint8)
                pts = cv2.findNonZero(empty)
                if pts is not None:
                    min_cov = cov
                    best_cell = (cx0, cy0)
                    best_pts = pts

    if best_pts is None:
        return None

    avg = best_pts.mean(axis=0)[0]
    return (int(avg[0]) + best_cell[0], int(avg[1]) + best_cell[1])


def inference_worker(model):
    global latest_pos, latest_coverage
    logging.info("Inference worker started")
    while not stop_event.is_set():
        try:
            frame = frame_q.get(timeout=0.5)
        except Empty:
            continue
        h, w = frame.shape[:2]
        results = None
        try:
            results = model.predict(frame, imgsz=IMGSZ, conf=CONF_THRESHOLD, device=0, stream=False, verbose=False)
        except Exception as e:
            logging.warning(f"Inference error: {e}")
            results = None

        full_mask = build_full_mask_from_results(results, w, h)
        coverage, has_outside = calculate_coverage_and_outside(full_mask, FEED_AREA_RECT)
        next_pt = suggest_next_point(full_mask, FEED_AREA_RECT)

        with pos_lock:
            latest_coverage = coverage
            latest_pos = next_pt

        # annotate frame
        vis = frame.copy()
        # overlay mask (semi-transparent)
        colored = np.zeros_like(vis)
        colored[full_mask == 255] = (0, 255, 0)
        vis = cv2.addWeighted(vis, 1.0, colored, 0.45, 0)

        x0, y0, x1, y1 = FEED_AREA_RECT
        cv2.rectangle(vis, (x0, y0), (x1, y1), (255, 0, 0), 2)
        vis = draw_chinese_text_on_frame(vis, f"Coverage: {coverage:.2f}%", (x1 + 8, y1 + 8), font_size=20, color=(0,255,0))

        # 【新增】显示当前串口状态（方便调试）
        # state_text = {1: "等待发送手动坐标1", 2: "等待发送手动坐标2", 3: "自动路径规划"}.get(SERIAL_STATE, "未知状态")
        # vis = draw_chinese_text_on_frame(vis, f"状态: {state_text}", (10, 40), font_size=20, color=(255,255,0))

        if next_pt is not None:
            cv2.circle(vis, next_pt, 8, (255,0,0), -1)
            cv2.circle(vis, next_pt, 12, (255,255,255), 2)

        if has_outside:
            ay, ax = ALARM_SIGNAL_POS
            try:
                vis[ay, ax] = (0,0,255)
            except Exception:
                pass
            vis = draw_chinese_text_on_frame(vis, "警告: 框外有撒料!", (10, 10), font_size=26, color=(0,0,255))

        # 撒料完成显示
        if coverage >= END_THRESHOLD:
            vis = draw_chinese_text_on_frame(vis, "撒料完成！", (WIDTH//2 - 80, HEIGHT//2 - 20), font_size=32, color=(0,255,255))

        # push annotated frame to out_q (for ffmpeg)
        try:
            if not out_q.full():
                out_q.put_nowait(vis)
        except Exception:
            pass

    logging.info("Inference worker stopped")


def camera_worker(device_idx=0):
    logging.info("Camera worker started")
    cap = cv2.VideoCapture(device_idx)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, HEIGHT)
    cap.set(cv2.CAP_PROP_FPS, FPS)
    if not cap.isOpened():
        logging.error("Could not open camera")
        stop_event.set()
        return
    try:
        while not stop_event.is_set():
            ret, frame = cap.read()
            if not ret or frame is None:
                logging.warning("camera read failed")
                time.sleep(0.1)
                continue
            # ensure correct size
            if frame.shape[1] != WIDTH or frame.shape[0] != HEIGHT:
                frame = cv2.resize(frame, (WIDTH, HEIGHT))
            try:
                if not frame_q.full():
                    frame_q.put_nowait(frame)
            except Exception:
                pass
            time.sleep(1.0 / FPS)
    finally:
        try:
            cap.release()
        except Exception:
            pass
        logging.info("Camera worker stopped")


def ffmpeg_worker():
    logging.info("FFmpeg worker starting")
    try:
        proc = subprocess.Popen(FFMPEG_CMD, stdin=subprocess.PIPE)
    except Exception as e:
        logging.error(f"Start ffmpeg failed: {e}")
        return
    try:
        while not stop_event.is_set():
            try:
                frame = out_q.get(timeout=0.5)
            except Empty:
                continue
            try:
                proc.stdin.write(frame.tobytes())
            except BrokenPipeError:
                logging.warning("ffmpeg broken pipe, restarting")
                proc.terminate()
                try:
                    proc.wait(timeout=1)
                except Exception:
                    pass
                try:
                    proc = subprocess.Popen(FFMPEG_CMD, stdin=subprocess.PIPE)
                except Exception as e:
                    logging.error(f"Restart ffmpeg failed: {e}")
                    time.sleep(1)
            except Exception as e:
                logging.debug(f"ffmpeg write error: {e}")
    finally:
        try:
            if proc.stdin:
                proc.stdin.close()
        except Exception:
            pass
        try:
            proc.terminate()
            proc.wait(timeout=1)
        except Exception:
            pass
        logging.info("FFmpeg worker stopped")


def serial_thread_fn(port, baud, timeout):
    global SERIAL_STATE  # 引用全局状态变量
    logging.info(f"Serial thread opening {port}@{baud}")
    try:
        ser = serial.Serial(port, baud, timeout=timeout)
    except Exception as e:
        logging.error(f"Open serial failed: {e}")
        return
    try:
        while not stop_event.is_set():
            try:
                raw = ser.readline()
            except Exception:
                break
            if not raw:
                # 覆盖率达标时自动发end（原有逻辑不变）
                with pos_lock:
                    cov = latest_coverage
                if cov >= END_THRESHOLD:
                    try:
                        ser.write(b'end\r\n')
                        ser.flush()
                        logging.info("Serial TX: end (auto)")
                        time.sleep(0.5)
                    except Exception as e:
                        logging.debug(f"serial write failed: {e}")
                time.sleep(0.05)
                continue

            # 处理收到的串口数据（核心修改：按状态发送坐标）
            try:
                line = raw.decode(errors='ignore').strip()
            except Exception:
                line = raw.strip().decode('latin1', errors='ignore')
            if not line:
                continue
            logging.debug(f"Serial RX: {line}")

            # 仅在收到"ok"时执行坐标发送逻辑
            if line.lower() == 'ok':
                # 1. 状态1：发送第一个手动指定坐标
                if SERIAL_STATE == 1:
                    x, y = MANUAL_POINT_1
                    # 坐标处理（与自动逻辑一致：缩放、校验）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 2  # 切换到下一个状态
                    logging.info(f"手动坐标1发送完成，等待下一个OK")

                # 2. 状态2：发送第二个手动指定坐标
                elif SERIAL_STATE == 2:
                    x, y = MANUAL_POINT_2
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 3  # 切换到自动规划状态
                    logging.info(f"手动坐标2发送完成")

                elif SERIAL_STATE == 3:
                    x, y = MANUAL_POINT_3
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 4  # 切换到自动规划状态
                    logging.info(f"手动坐标3发送完成")

                elif SERIAL_STATE == 4:
                    x, y = MANUAL_POINT_4
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 5  # 切换到自动规划状态
                    logging.info(f"手动坐标4发送完成")

                elif SERIAL_STATE == 5:
                    x, y = MANUAL_POINT_5
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 6  # 切换到自动规划状态
                    logging.info(f"手动坐标5发送完成")

                elif SERIAL_STATE == 6:
                    x, y = MANUAL_POINT_6
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 7  # 切换到自动规划状态
                    logging.info(f"手动坐标6发送完成")

                elif SERIAL_STATE == 7:
                    x, y = MANUAL_POINT_7
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 8  # 切换到自动规划状态
                    logging.info(f"手动坐标7发送完成")

                elif SERIAL_STATE == 8:
                    x, y = MANUAL_POINT_8
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 9  # 切换到自动规划状态
                    logging.info(f"手动坐标8发送完成")

                elif SERIAL_STATE == 9:
                    x, y = MANUAL_POINT_9
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 10  # 切换到自动规划状态
                    logging.info(f"手动坐标9发送完成")

                elif SERIAL_STATE == 10:
                    x, y = MANUAL_POINT_10
                    # 坐标处理（与自动逻辑一致）
                    x_scaled = int(round(max(0, min(x, WIDTH)) / 20))
                    y_scaled = int(round(max(0, min(y, HEIGHT)) / 20))
                    out = f'X{x_scaled}Y{y_scaled}\r\n'
                    SERIAL_STATE = 11  # 切换到自动规划状态
                    logging.info(f"手动坐标10发送完成，进入自动路径规划")

                # 状态11：执行原有自动路径规划逻辑
                elif SERIAL_STATE == 11:
                    with pos_lock:
                        pos = latest_pos
                        cov = latest_coverage
                    # 覆盖率达标发end，否则发自动计算的坐标
                    if cov >= END_THRESHOLD:
                        out = 'end\r\n'
                    else:
                        if pos is None:
                            out = 'NA\r\n'
                        else:
                            x_scaled = int(round(max(0, min(pos[0], WIDTH)) / 20))
                            y_scaled = int(round(max(0, min(pos[1], HEIGHT)) / 20))
                            out = f'X{x_scaled}Y{y_scaled}\r\n'

                # 发送坐标（所有状态的发送逻辑统一）
                try:
                    ser.write(out.encode())
                    ser.flush()
                    logging.info(f"Serial TX: {out.strip()}")
                except Exception as e:
                    logging.error(f"Serial send failed: {e}")
            time.sleep(0.02)
    finally:
        try:
            ser.close()
        except Exception:
            pass
        logging.info("Serial thread stopped")


def start_mediamtx():
    # Try start mediamtx if present
    try:
        proc = subprocess.Popen(['./mediamtx'])
        time.sleep(1.0)
        logging.info('mediamtx started')
        return proc
    except Exception:
        logging.info('mediamtx not started (not found or failed)')
        return None


def main():
    logging.info('===== saliao optimized starting =====')
    mtx = start_mediamtx()

    # load model
    model = None
    try:
        model = YOLO(MODEL_PATH, task=TASK)
        logging.info('model loaded')
    except Exception as e:
        logging.error(f'model load failed: {e}')

    # threads（相机、推理、推流线程逻辑不变）
    cam_t = threading.Thread(target=camera_worker, args=(CAMERA_DEVICE,), daemon=True)
    inf_t = threading.Thread(target=inference_worker, args=(model,), daemon=True)
    ffm_t = threading.Thread(target=ffmpeg_worker, daemon=True)
    ser_t = threading.Thread(target=serial_thread_fn, args=(SERIAL_PORT, SERIAL_BAUD, SERIAL_TIMEOUT), daemon=True)

    cam_t.start()
    inf_t.start()
    ffm_t.start()
    ser_t.start()

    try:
        while True:
            time.sleep(0.5)
    except KeyboardInterrupt:
        logging.info('KeyboardInterrupt received, exiting')
    finally:
        stop_event.set()
        time.sleep(0.5)
        logging.info('Shutdown complete')


if __name__ == '__main__':
    main()