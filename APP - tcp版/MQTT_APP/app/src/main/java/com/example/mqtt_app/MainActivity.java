//package com.example.mqtt_app;
//
//import androidx.appcompat.app.AppCompatActivity;
//import androidx.core.content.ContextCompat;
//
//import android.annotation.SuppressLint;
//import android.graphics.Color;
//import android.graphics.drawable.Drawable;
//import android.os.Bundle;
//import android.os.Handler;
//import android.os.Message;
//import android.util.Log;
//import android.view.View;
//import android.widget.Button;
//import android.widget.ImageView;
//import android.widget.TextView;
//import android.widget.Toast;
//import android.widget.ToggleButton;
//
//import org.json.JSONException;
//import org.json.JSONObject;
//
//import java.io.BufferedReader;
//import java.io.DataOutputStream;
//import java.io.InputStreamReader;
//import java.net.ServerSocket;
//import java.net.Socket;
//import java.util.concurrent.ExecutorService;
//import java.util.concurrent.Executors;
//
//public class MainActivity extends AppCompatActivity {
//
//    private ServerSocket serverSocket;
//    private Socket clientSocket;
//    private DataOutputStream outputStream;
//    private BufferedReader inputStream;
//    private ExecutorService executorService;
//    private Handler handler;
//    private boolean isServerRunning = false;
//
//    private UDPClient udpClient = new UDPClient();
//    private ImageView moveHead;
//    private boolean isStreaming = false;
//
//    // TCP服务器配置
//    private int serverPort = 8086;              // TCP服务器端口号
//
//    // UI组件
//    private Button btnForward, btnBackward, btnLeft, btnRight;
//    private TextView btnStop, btnStart;  // 改为TextView类型
//    private ToggleButton btnPower;
//    private TextView tvStatus;
//
//    // 移动平台当前位置和边界限制
//    private float currentX = 0f;
//    private float currentY = 0f;
//    private static final float MOVE_DISTANCE = 30f;
//    private static final float MAX_X = 150f;  // X方向最大位移
//    private static final float MAX_Y = 150f;  // Y方向最大位移
//
//    @Override
//    protected void onCreate(Bundle savedInstanceState) {
//        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);
//
//        // 初始化UI组件
//        initUI();
//
//        // 设置按钮背景颜色
//        setButtonColors();
//
//        // 设置按钮点击事件
//        setButtonListeners();
//
//        // 初始化TCP服务器
//        tcp_init();
//
//        handler = new Handler() {
//            @SuppressLint("SetTextI18n")
//            public void handleMessage(Message msg) {
//                super.handleMessage(msg);
//                switch (msg.what) {
//                    case 1: //开机校验更新回传
//                        break;
//                    case 2:  // 反馈回传
//                        break;
//                    case 3:  //TCP 收到消息回传
//                        String message = msg.obj.toString();
//                        Log.d("nicecode", "handleMessage: " + message);
//                        try {
//                            JSONObject jsonObjectALL = new JSONObject(message);
//
//                            // 检查是否存在items字段
//                            if (jsonObjectALL.has("items")) {
//                                JSONObject items = jsonObjectALL.getJSONObject("items");
//
//                                // 处理设备状态更新
//                                try {
//                                    // 检查是否存在status字段
//                                    if (items.has("status")) {
//                                        JSONObject objStatus = items.getJSONObject("status");
//                                        String statusValue = objStatus.getString("value");
//                                        tvStatus.setText(statusValue);
//                                    }
//                                    // 可以在这里添加对其他字段的处理
//                                } catch (JSONException e) {
//                                    Log.e("nicecode", "状态解析错误: " + e.getMessage());
//                                    // 不显示Toast，避免干扰用户
//                                }
//                            } else {
//                                // 如果没有items字段，可能是其他类型的消息
//                                Log.d("nicecode", "收到不包含items字段的消息: " + message);
//                                // 可以在这里处理其他类型的消息
//                            }
//                        } catch (JSONException e) {
//                            Log.e("nicecode", "JSON解析失败: " + e.getMessage() + ", 消息内容: " + message);
//                            // 不显示Toast，避免干扰用户
//                        }
//                        break;
//                    case 30:  //服务器启动失败
//                        Toast.makeText(MainActivity.this, "TCP服务器启动失败", Toast.LENGTH_SHORT).show();
//                        tvStatus.setText("服务器启动失败");
//                        break;
//                    case 31:   //服务器启动成功
//                        Toast.makeText(MainActivity.this, "TCP服务器启动成功", Toast.LENGTH_SHORT).show();
//                        tvStatus.setText("服务器运行中");
//                        break;
//                    case 32:   //客户端连接成功
//                        Toast.makeText(MainActivity.this, "客户端连接成功", Toast.LENGTH_SHORT).show();
//                        tvStatus.setText("客户端已连接");
//                        break;
//                    case 33:   //客户端断开连接
//                        Toast.makeText(MainActivity.this, "客户端断开连接", Toast.LENGTH_SHORT).show();
//                        tvStatus.setText("等待客户端连接");
//                        break;
//                    default:
//                        break;
//                }
//            }
//        };
//    }
//
//    private void initUI() {
//        moveHead = findViewById(R.id.move_head);
//        btnForward = findViewById(R.id.btn_forward);
//        btnBackward = findViewById(R.id.btn_backward);
//        btnLeft = findViewById(R.id.btn_left);
//        btnRight = findViewById(R.id.btn_right);
//        btnStop = findViewById(R.id.btn_stop);
//        btnStart = findViewById(R.id.btn_start);
//        btnPower = findViewById(R.id.btn_power);
//        tvStatus = findViewById(R.id.tv_status);
//
//        // 添加测试按钮点击事件，用于测试TCP服务器
//        tvStatus.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                testTCPServer();
//            }
//        });
//    }
//
//    private void testTCPServer() {
//        Toast.makeText(this, "TCP服务器状态: " + (isServerRunning ? "运行中" : "未运行"), Toast.LENGTH_SHORT).show();
//    }
//
//    private void setButtonColors() {
//        // 设置停止和开始按钮的背景
//        btnStop.setBackgroundResource(R.drawable.btn_stop_rounded);
//        btnStart.setBackgroundResource(R.drawable.btn_start_rounded);
//
//        // 设置文字颜色
//        btnStop.setTextColor(Color.WHITE);
//        btnStart.setTextColor(Color.WHITE);
//    }
//
//    private void setButtonListeners() {
//        // 方向控制按钮 (调整位置：上下为前后，左右为左右)
//        btnForward.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                System.out.println("点击前进按钮");
//                sendControlMessage("forward");
//                // 移动平台沿Y正方向移动（前）
//                if (currentY - MOVE_DISTANCE >= -MAX_Y) {
//                    moveHead.animate().translationYBy(-MOVE_DISTANCE).setDuration(300);
//                    currentY -= MOVE_DISTANCE;
//                } else {
//                    Toast.makeText(MainActivity.this, "已到达Y轴正向边界", Toast.LENGTH_SHORT).show();
//                }
//            }
//        });
//
//        btnBackward.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                System.out.println("点击后退按钮");
//                sendControlMessage("backward");
//                // 移动平台沿Y负方向移动（后）
//                if (currentY + MOVE_DISTANCE <= MAX_Y) {
//                    moveHead.animate().translationYBy(MOVE_DISTANCE).setDuration(300);
//                    currentY += MOVE_DISTANCE;
//                } else {
//                    Toast.makeText(MainActivity.this, "已到达Y轴负向边界", Toast.LENGTH_SHORT).show();
//                }
//            }
//        });
//
//        btnLeft.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                System.out.println("点击左转按钮");
//                sendControlMessage("left");
//                // 移动平台沿X负方向移动（左）
//                if (currentX - MOVE_DISTANCE >= -MAX_X) {
//                    moveHead.animate().translationXBy(-MOVE_DISTANCE).setDuration(300);
//                    currentX -= MOVE_DISTANCE;
//                } else {
//                    Toast.makeText(MainActivity.this, "已到达X轴负向边界", Toast.LENGTH_SHORT).show();
//                }
//            }
//        });
//
//        btnRight.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                System.out.println("点击右转按钮");
//                sendControlMessage("right");
//                // 移动平台沿X正方向移动（右）
//                if (currentX + MOVE_DISTANCE <= MAX_X) {
//                    moveHead.animate().translationXBy(MOVE_DISTANCE).setDuration(300);
//                    currentX += MOVE_DISTANCE;
//                } else {
//                    Toast.makeText(MainActivity.this, "已到达X轴正向边界", Toast.LENGTH_SHORT).show();
//                }
//            }
//        });
//
//        // 功能按钮
//        btnStop.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                System.out.println("点击停止按钮");
//                sendControlMessage("stop");
//                Toast.makeText(MainActivity.this, "发送停止指令", Toast.LENGTH_SHORT).show();
//            }
//        });
//
//        btnStart.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                System.out.println("点击开始按钮");
//                sendControlMessage("start");
//                Toast.makeText(MainActivity.this, "发送开始指令", Toast.LENGTH_SHORT).show();
//            }
//        });
//
//        btnPower.setOnCheckedChangeListener((buttonView, isChecked) -> {
//            System.out.println("电源开关状态改变: " + isChecked);
//            if (isChecked) {
//                sendControlMessage("power_on");
//                tvStatus.setText("撒料口已开启");
//                Toast.makeText(MainActivity.this, "撒料口已开启", Toast.LENGTH_SHORT).show();
//            } else {
//                sendControlMessage("power_off");
//                tvStatus.setText("撒料口已关闭");
//                Toast.makeText(MainActivity.this, "撒料口已关闭", Toast.LENGTH_SHORT).show();
//            }
//        });
//    }
//
//    private void sendControlMessage(String command) {
//        String message = "";
//
//        switch(command) {
//            case "forward":
//                // TSL：btn_forward（int，0-10），传1符合范围
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_forward\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "backward":
//                // TSL：btn_backward（int，0-10）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_backward\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "left":
//                // 若按方案A修改TSL为rw，保留此段；若按方案B，删除此段（不发送消息）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_left\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "right":
//                // TSL：btn_right（int，0-10）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_right\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "stop":
//                // TSL：btn_stop（bool，0=关，1=开），传1代表触发停止
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_stop\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "start":
//                // TSL：btn_start（bool，0=关，1=开）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_start\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "power_on":
//                // TSL：btn_power（bool，1=开）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_power\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "power_off":
//                // TSL：btn_power（bool，0=关）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_power\": 0\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            default:
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"control_command\": \"" + command + "\"\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//        }
//
//        publish_message(message);
//    }
//
//    private void tcp_init() {
//        executorService = Executors.newSingleThreadExecutor();
//        startTCPServer();
//    }
//
//    private void startTCPServer() {
//        executorService.execute(() -> {
//            try {
//                System.out.println("【TCP服务器】尝试启动服务器，端口: " + serverPort);
//                serverSocket = new ServerSocket(serverPort);
//                isServerRunning = true;
//                System.out.println("【TCP服务器】启动成功，监听端口: " + serverPort);
//
//                // 通知UI服务器启动成功
//                Message msg = new Message();
//                msg.what = 31;
//                handler.sendMessage(msg);
//
//                // 等待客户端连接
//                while (isServerRunning) {
//                    try {
//                        System.out.println("【TCP服务器】等待客户端连接...");
//                        clientSocket = serverSocket.accept();
//                        System.out.println("【TCP服务器】客户端连接成功: " + clientSocket.getInetAddress());
//
//                        // 通知UI客户端连接成功
//                        Message connectMsg = new Message();
//                        connectMsg.what = 32;
//                        handler.sendMessage(connectMsg);
//
//                        // 初始化输入输出流
//                        outputStream = new DataOutputStream(clientSocket.getOutputStream());
//                        inputStream = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
//
//                        // 发送欢迎消息给客户端
//                        sendWelcomeMessage();
//
//                        // 启动接收消息线程
//                        startReceiving();
//
//                    } catch (Exception e) {
//                        if (isServerRunning) {
//                            System.err.println("【TCP服务器】接受客户端连接时出错: " + e.getMessage());
//                            e.printStackTrace();
//                        }
//                    }
//                }
//            } catch (Exception e) {
//                System.err.println("【TCP服务器】启动TCP服务器时出错: " + e.getMessage());
//                e.printStackTrace();
//
//                // 通知UI服务器启动失败
//                Message msg = new Message();
//                msg.what = 30;
//                handler.sendMessage(msg);
//            }
//        });
//    }
//
//    private void sendWelcomeMessage() {
//        System.out.println("【TCP服务器】准备发送欢迎消息");
//        executorService.execute(() -> {
//            try {
//                // 等待一小段时间确保连接完全建立
//                Thread.sleep(100);
//
//                if (outputStream != null && clientSocket != null && !clientSocket.isClosed()) {
//                    String welcomeMessage = "欢迎连接到TCP服务器!\n";
//                    System.out.println("【TCP服务器】发送欢迎消息: " + welcomeMessage.trim());
//                    // 使用write()方法发送消息
//                    outputStream.write(welcomeMessage.getBytes("UTF-8"));
//                    outputStream.flush();
//                    System.out.println("【TCP服务器】欢迎消息发送成功");
//                } else {
//                    System.err.println("【TCP服务器】无法发送欢迎消息 - 连接已关闭或输出流为空");
//                }
//            } catch (Exception e) {
//                System.err.println("【TCP服务器】发送欢迎消息时出错: " + e.getMessage());
//                e.printStackTrace();
//            }
//        });
//    }
//
//    private void startReceiving() {
//        executorService.execute(() -> {
//            try {
//                if (inputStream != null) {
//                    System.out.println("【TCP服务器】启动消息接收线程");
//                    String message;
//                    while (clientSocket != null && !clientSocket.isClosed() && (message = inputStream.readLine()) != null) {
//                        System.out.println("【TCP服务器】收到客户端消息: " + message);
//
//                        // 处理收到的消息
//                        Message msg = new Message();
//                        msg.what = 3;
//                        msg.obj = message;
//                        handler.sendMessage(msg);
//                    }
//                    System.out.println("【TCP服务器】消息接收线程结束");
//                }
//
//                // 客户端断开连接
//                System.out.println("【TCP服务器】客户端断开连接");
//                // 清理资源
//                cleanupClientResources();
//
//                Message disconnectMsg = new Message();
//                disconnectMsg.what = 33;
//                handler.sendMessage(disconnectMsg);
//
//            } catch (Exception e) {
//                System.err.println("【TCP服务器】接收客户端消息时出错: " + e.getMessage());
//                e.printStackTrace();
//                // 清理资源
//                cleanupClientResources();
//            }
//        });
//    }
//
//    private void cleanupClientResources() {
//        try {
//            if (outputStream != null) {
//                outputStream.close();
//                outputStream = null;
//            }
//            if (inputStream != null) {
//                inputStream.close();
//                inputStream = null;
//            }
//            if (clientSocket != null) {
//                clientSocket.close();
//                clientSocket = null;
//            }
//            System.out.println("客户端资源已清理");
//        } catch (Exception e) {
//            System.err.println("清理客户端资源时出错: " + e.getMessage());
//            e.printStackTrace();
//        }
//    }
//
//    private void publish_message(String message) {
//        System.out.println("【TCP服务器】准备发送消息: " + message);
//        if (clientSocket == null) {
//            System.err.println("【TCP服务器】客户端Socket为空，无法发送消息");
//            Toast.makeText(MainActivity.this, "客户端未连接", Toast.LENGTH_SHORT).show();
//            return;
//        }
//
//        if (clientSocket.isClosed()) {
//            System.err.println("【TCP服务器】客户端Socket已关闭，无法发送消息");
//            Toast.makeText(MainActivity.this, "客户端连接已断开", Toast.LENGTH_SHORT).show();
//            return;
//        }
//
//        if (outputStream == null) {
//            System.err.println("【TCP服务器】输出流为空，无法发送消息");
//            Toast.makeText(MainActivity.this, "输出流未初始化", Toast.LENGTH_SHORT).show();
//            return;
//        }
//
//        System.out.println("【TCP服务器】客户端已连接，开始发送消息");
//        executorService.execute(() -> {
//            try {
//                System.out.println("【TCP服务器】在发送线程中，准备写入消息到输出流");
//                // 使用write()方法发送消息，并手动添加换行符
//                outputStream.write((message + "\n").getBytes("UTF-8"));
//                outputStream.flush();
//                System.out.println("【TCP服务器】消息发送成功: " + message);
//            } catch (Exception e) {
//                System.err.println("【TCP服务器】发送消息时出错: " + e.getMessage());
//                e.printStackTrace();
//                // 在主线程中显示错误信息
//                runOnUiThread(() -> {
//                    Toast.makeText(MainActivity.this, "消息发送失败: " + e.getMessage(), Toast.LENGTH_SHORT).show();
//                });
//            }
//        });
//    }
//
//    @Override
//    protected void onDestroy() {
//        super.onDestroy();
//        udpClient.stopReceiving();
//
//        // 停止TCP服务器
//        isServerRunning = false;
//        cleanupClientResources();
//
//        try {
//            if (serverSocket != null) {
//                serverSocket.close();
//            }
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
//
//        if (executorService != null) {
//            executorService.shutdown();
//        }
//    }
//}

package com.example.mqtt_app;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import android.annotation.SuppressLint;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {

    private ServerSocket serverSocket;
    private Socket clientSocket;
    private DataOutputStream outputStream;
    private BufferedReader inputStream;
    private ExecutorService executorService;
    private Handler handler;
    private boolean isServerRunning = false;

    private UDPClient udpClient = new UDPClient();
    private ImageView moveHead;
    private boolean isStreaming = false;

    // TCP服务器配置
    private int serverPort = 8086;              // TCP服务器端口号

    // UI组件
    private Button btnForward, btnBackward, btnLeft, btnRight;
    private TextView btnStop, btnStart;  // 改为TextView类型
    private ToggleButton btnPower;
    private TextView tvStatus;

    // 移动平台当前位置和边界限制
    private float currentX = 0f;
    private float currentY = 0f;
    private static final float MOVE_DISTANCE = 30f;
    private static final float MAX_X = 150f;  // X方向最大位移
    private static final float MAX_Y = 150f;  // Y方向最大位移

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // 初始化UI组件
        initUI();

        // 设置按钮背景颜色
        setButtonColors();

        // 设置按钮点击事件
        setButtonListeners();

        // 初始化TCP服务器
        tcp_init();

        handler = new Handler() {
            @SuppressLint("SetTextI18n")
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case 1: //开机校验更新回传
                        break;
                    case 2:  // 反馈回传
                        break;
                    case 3:  //TCP 收到消息回传
                        String message = msg.obj.toString();
                        Log.d("nicecode", "handleMessage: " + message);
                        try {
                            JSONObject jsonObjectALL = new JSONObject(message);

                            // 检查是否存在items字段
                            if (jsonObjectALL.has("items")) {
                                JSONObject items = jsonObjectALL.getJSONObject("items");

                                // 处理设备状态更新
                                try {
                                    // 检查是否存在status字段
                                    if (items.has("status")) {
                                        JSONObject objStatus = items.getJSONObject("status");
                                        String statusValue = objStatus.getString("value");
                                        tvStatus.setText(statusValue);
                                    }
                                    // 可以在这里添加对其他字段的处理
                                } catch (JSONException e) {
                                    Log.e("nicecode", "状态解析错误: " + e.getMessage());
                                    // 不显示Toast，避免干扰用户
                                }
                            } else {
                                // 如果没有items字段，可能是其他类型的消息
                                Log.d("nicecode", "收到不包含items字段的消息: " + message);
                                // 可以在这里处理其他类型的消息
                            }
                        } catch (JSONException e) {
                            Log.e("nicecode", "JSON解析失败: " + e.getMessage() + ", 消息内容: " + message);
                            // 不显示Toast，避免干扰用户
                        }
                        break;
                    case 30:  //服务器启动失败
                        Toast.makeText(MainActivity.this, "TCP服务器启动失败", Toast.LENGTH_SHORT).show();
                        tvStatus.setText("服务器启动失败");
                        break;
                    case 31:   //服务器启动成功
                        Toast.makeText(MainActivity.this, "TCP服务器启动成功", Toast.LENGTH_SHORT).show();
                        tvStatus.setText("服务器运行中");
                        break;
                    case 32:   //客户端连接成功
                        Toast.makeText(MainActivity.this, "客户端连接成功", Toast.LENGTH_SHORT).show();
                        tvStatus.setText("客户端已连接");
                        break;
                    case 33:   //客户端断开连接
                        Toast.makeText(MainActivity.this, "客户端断开连接", Toast.LENGTH_SHORT).show();
                        tvStatus.setText("等待客户端连接");
                        break;
                    default:
                        break;
                }
            }
        };
    }

    private void initUI() {
        moveHead = findViewById(R.id.move_head);
        btnForward = findViewById(R.id.btn_forward);
        btnBackward = findViewById(R.id.btn_backward);
        btnLeft = findViewById(R.id.btn_left);
        btnRight = findViewById(R.id.btn_right);
        btnStop = findViewById(R.id.btn_stop);
        btnStart = findViewById(R.id.btn_start);
        btnPower = findViewById(R.id.btn_power);
        tvStatus = findViewById(R.id.tv_status);

        // 添加测试按钮点击事件，用于测试TCP服务器
        tvStatus.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                testTCPServer();
            }
        });
    }

    private void testTCPServer() {
        Toast.makeText(this, "TCP服务器状态: " + (isServerRunning ? "运行中" : "未运行"), Toast.LENGTH_SHORT).show();
    }

    private void setButtonColors() {
        // 设置停止和开始按钮的背景
        btnStop.setBackgroundResource(R.drawable.btn_stop_rounded);
        btnStart.setBackgroundResource(R.drawable.btn_start_rounded);

        // 设置文字颜色
        btnStop.setTextColor(Color.WHITE);
        btnStart.setTextColor(Color.WHITE);
    }

    private void setButtonListeners() {
        // 方向控制按钮 (调整位置：上下为前后，左右为左右)
        btnForward.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                System.out.println("点击前进按钮");
                sendControlMessage("forward");
                // 移动平台沿Y正方向移动（前）
                if (currentY - MOVE_DISTANCE >= -MAX_Y) {
                    moveHead.animate().translationYBy(-MOVE_DISTANCE).setDuration(300);
                    currentY -= MOVE_DISTANCE;
                } else {
                    Toast.makeText(MainActivity.this, "已到达Y轴正向边界", Toast.LENGTH_SHORT).show();
                }
            }
        });

        btnBackward.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                System.out.println("点击后退按钮");
                sendControlMessage("backward");
                // 移动平台沿Y负方向移动（后）
                if (currentY + MOVE_DISTANCE <= MAX_Y) {
                    moveHead.animate().translationYBy(MOVE_DISTANCE).setDuration(300);
                    currentY += MOVE_DISTANCE;
                } else {
                    Toast.makeText(MainActivity.this, "已到达Y轴负向边界", Toast.LENGTH_SHORT).show();
                }
            }
        });

        btnLeft.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                System.out.println("点击左转按钮");
                sendControlMessage("left");
                // 移动平台沿X负方向移动（左）
                if (currentX - MOVE_DISTANCE >= -MAX_X) {
                    moveHead.animate().translationXBy(-MOVE_DISTANCE).setDuration(300);
                    currentX -= MOVE_DISTANCE;
                } else {
                    Toast.makeText(MainActivity.this, "已到达X轴负向边界", Toast.LENGTH_SHORT).show();
                }
            }
        });

        btnRight.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                System.out.println("点击右转按钮");
                sendControlMessage("right");
                // 移动平台沿X正方向移动（右）
                if (currentX + MOVE_DISTANCE <= MAX_X) {
                    moveHead.animate().translationXBy(MOVE_DISTANCE).setDuration(300);
                    currentX += MOVE_DISTANCE;
                } else {
                    Toast.makeText(MainActivity.this, "已到达X轴正向边界", Toast.LENGTH_SHORT).show();
                }
            }
        });

        // 功能按钮
        btnStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                System.out.println("点击停止按钮");
                sendControlMessage("stop");
                Toast.makeText(MainActivity.this, "发送停止指令", Toast.LENGTH_SHORT).show();
            }
        });

        btnStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                System.out.println("点击开始按钮");
                sendControlMessage("start");
                Toast.makeText(MainActivity.this, "发送开始指令", Toast.LENGTH_SHORT).show();
            }
        });

        btnPower.setOnCheckedChangeListener((buttonView, isChecked) -> {
            System.out.println("电源开关状态改变: " + isChecked);
            if (isChecked) {
                sendControlMessage("power_on");
                tvStatus.setText("撒料口已开启");
                Toast.makeText(MainActivity.this, "撒料口已开启", Toast.LENGTH_SHORT).show();
            } else {
                sendControlMessage("power_off");
                tvStatus.setText("撒料口已关闭");
                Toast.makeText(MainActivity.this, "撒料口已关闭", Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void sendControlMessage(String command) {
        String message = "";

        switch (command) {
            case "forward":
                message = "forward";
                break;
            case "backward":
                message = "backward";
                break;
            case "left":
                message = "left";
                break;
            case "right":
                message = "right";
                break;
            case "stop":
                message = "stop";
                break;
            case "start":
                message = "start";
                break;
            case "power_on":
                message = "power_on";
                break;
            case "power_off":
                message = "power_off";
                break;
            default:
                message = command; // 直接发送未知指令文本
                break;
        }
//        switch(command) {
//            case "forward":
//                // TSL：btn_forward（int，0-10），传1符合范围
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_forward\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "backward":
//                // TSL：btn_backward（int，0-10）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_backward\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "left":
//                // 若按方案A修改TSL为rw，保留此段；若按方案B，删除此段（不发送消息）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_left\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "right":
//                // TSL：btn_right（int，0-10）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_right\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "stop":
//                // TSL：btn_stop（bool，0=关，1=开），传1代表触发停止
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_stop\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "start":
//                // TSL：btn_start（bool，0=关，1=开）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_start\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "power_on":
//                // TSL：btn_power（bool，1=开）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_power\": 1\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            case "power_off":
//                // TSL：btn_power（bool，0=关）
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"btn_power\": 0\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//            default:
//                message = "{\n" +
//                        "  \"method\": \"thing.event.property.post\",\n" +
//                        "  \"id\": \"12345\",\n" +
//                        "  \"params\": {\n" +
//                        "    \"control_command\": \"" + command + "\"\n" +
//                        "  },\n" +
//                        "  \"version\": \"1.0\"\n" +
//                        "}";
//                break;
//        }

        publish_message(message);
    }

    private void tcp_init() {
        executorService = Executors.newFixedThreadPool(4);
        startTCPServer();
    }

    private void startTCPServer() {
        executorService.execute(() -> {
            try {
                System.out.println("【TCP服务器】尝试启动服务器，端口: " + serverPort);
                serverSocket = new ServerSocket(serverPort);
                isServerRunning = true;
                System.out.println("【TCP服务器】启动成功，监听端口: " + serverPort);

                // 通知UI服务器启动成功
                Message msg = new Message();
                msg.what = 31;
                handler.sendMessage(msg);

                // 等待客户端连接
                while (isServerRunning) {
                    try {
                        System.out.println("【TCP服务器】等待客户端连接...");
                        clientSocket = serverSocket.accept();
                        System.out.println("【TCP服务器】客户端连接成功: " + clientSocket.getInetAddress());

                        // 设置Socket选项，提高实时性
                        clientSocket.setTcpNoDelay(true); // 禁用Nagle算法
                        clientSocket.setSoTimeout(0); // 无读取超时

                        // 通知UI客户端连接成功
                        Message connectMsg = new Message();
                        connectMsg.what = 32;
                        handler.sendMessage(connectMsg);

                        // 初始化输入输出流
                        outputStream = new DataOutputStream(clientSocket.getOutputStream());
                        inputStream = new BufferedReader(new InputStreamReader(clientSocket.getInputStream(), "UTF-8"));

                        // 发送欢迎消息给客户端
                        sendWelcomeMessage();

                        // 启动接收消息线程
                        startReceiving();

                    } catch (Exception e) {
                        if (isServerRunning) {
                            System.err.println("【TCP服务器】接受客户端连接时出错: " + e.getMessage());
                            e.printStackTrace();
                        }
                    }
                }
            } catch (Exception e) {
                System.err.println("【TCP服务器】启动TCP服务器时出错: " + e.getMessage());
                e.printStackTrace();

                // 通知UI服务器启动失败
                Message msg = new Message();
                msg.what = 30;
                handler.sendMessage(msg);
            }
        });
    }

    private void sendWelcomeMessage() {
        System.out.println("【TCP服务器】准备发送欢迎消息");
        executorService.execute(() -> {
            try {
                // 等待一小段时间确保连接完全建立
                Thread.sleep(100);

                if (isClientConnected() && outputStream != null) {
                    // String welcomeMessage = "欢迎连接到TCP服务器!连接时间: " + System.currentTimeMillis() + "\n";
                    // System.out.println("【TCP服务器】发送欢迎消息: " + welcomeMessage.trim());

                    // 使用write()方法发送消息，确保以换行符结尾
//                    outputStream.write(welcomeMessage.getBytes("UTF-8"));
//                    outputStream.flush();
                    System.out.println("【TCP服务器】欢迎消息发送成功");
                } else {
                    System.err.println("【TCP服务器】无法发送欢迎消息 - 连接已关闭或输出流为空");
                }
            } catch (Exception e) {
                System.err.println("【TCP服务器】发送欢迎消息时出错: " + e.getMessage());
                e.printStackTrace();
            }
        });
    }

    private void startReceiving() {
        executorService.execute(() -> {
            try {
                if (inputStream != null) {
                    System.out.println("【TCP服务器】启动消息接收线程");
                    String message;
                    while (isClientConnected() && (message = inputStream.readLine()) != null) {
                        System.out.println("【TCP服务器】收到客户端消息: " + message);

                        // 处理收到的消息
                        Message msg = new Message();
                        msg.what = 3;
                        msg.obj = message;
                        handler.sendMessage(msg);
                    }
                    System.out.println("【TCP服务器】消息接收线程结束");
                }

                // 客户端断开连接
                System.out.println("【TCP服务器】客户端断开连接");
                // 清理资源
                cleanupClientResources();

                Message disconnectMsg = new Message();
                disconnectMsg.what = 33;
                handler.sendMessage(disconnectMsg);

            } catch (Exception e) {
                System.err.println("【TCP服务器】接收客户端消息时出错: " + e.getMessage());
                e.printStackTrace();
                // 清理资源
                cleanupClientResources();
            }
        });
    }

    // 添加连接状态检查方法
    private boolean isClientConnected() {
        return clientSocket != null &&
                !clientSocket.isClosed() &&
                clientSocket.isConnected() &&
                !clientSocket.isInputShutdown() &&
                !clientSocket.isOutputShutdown();
    }

    private void cleanupClientResources() {
        try {
            if (outputStream != null) {
                outputStream.close();
                outputStream = null;
            }
            if (inputStream != null) {
                inputStream.close();
                inputStream = null;
            }
            if (clientSocket != null) {
                clientSocket.close();
                clientSocket = null;
            }
            System.out.println("客户端资源已清理");
        } catch (Exception e) {
            System.err.println("清理客户端资源时出错: " + e.getMessage());
            e.printStackTrace();
        }
    }

    private void publish_message(String message) {
        System.out.println("【TCP服务器】准备发送消息: " + message);

        // 检查连接状态
        if (!isClientConnected()) {
            System.err.println("【TCP服务器】客户端未连接或连接已关闭");
            runOnUiThread(() -> {
                Toast.makeText(MainActivity.this, "客户端未连接，无法发送消息", Toast.LENGTH_SHORT).show();
                tvStatus.setText("客户端未连接");
            });
            return;
        }

        if (outputStream == null) {
            System.err.println("【TCP服务器】输出流为空，无法发送消息");
            runOnUiThread(() -> {
                Toast.makeText(MainActivity.this, "输出流未初始化", Toast.LENGTH_SHORT).show();
            });
            return;
        }

        System.out.println("【TCP服务器】客户端已连接，开始发送消息");

        // 在lambda外部处理消息，确保变量是final或effectively final
        final String finalMessage = message.endsWith("\n") ? message : message + "\n";

        executorService.execute(() -> {
            try {
                System.out.println("【TCP服务器】发送消息内容: " + finalMessage.trim());

                // 使用UTF-8编码发送，确保编码一致
                byte[] messageBytes = finalMessage.getBytes("UTF-8");
                outputStream.write(messageBytes);
                outputStream.flush(); // 强制刷新缓冲区，确保数据立即发送

                System.out.println("【TCP服务器】消息发送成功，字节数: " + messageBytes.length);

            } catch (Exception e) {
                System.err.println("【TCP服务器】发送消息时出错: " + e.getMessage());
                e.printStackTrace();

                // 发生异常时清理连接
                cleanupClientResources();

                runOnUiThread(() -> {
                    Toast.makeText(MainActivity.this, "消息发送失败，连接可能已断开", Toast.LENGTH_SHORT).show();
                    tvStatus.setText("连接已断开");
                });
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        udpClient.stopReceiving();

        // 停止TCP服务器
        isServerRunning = false;
        cleanupClientResources();

        try {
            if (serverSocket != null) {
                serverSocket.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (executorService != null) {
            executorService.shutdown();
        }
    }
}