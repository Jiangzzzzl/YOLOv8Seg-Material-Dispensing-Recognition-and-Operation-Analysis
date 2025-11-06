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

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class MainActivity extends AppCompatActivity {

    private MqttClient client;
    private MqttConnectOptions options;
    private Handler handler;
    private ScheduledExecutorService scheduler;

    private UDPClient udpClient = new UDPClient();
    private ImageView moveHead;
    private boolean isStreaming = false;

    private String productKey = "k0ambjvkz9k";
    private String deviceName = "saliao_app";
    private String deviceSecret = "25245f864c87156b38220556c7208640";

    private final String pub_topic = "/sys/k0ambjvkz9k/saliao_app/thing/event/property/post";
    private final String sub_topic = "/sys/k0ambjvkz9k/saliao_app/thing/service/property/set";

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

        mqtt_init();
        start_reconnect();

        handler = new Handler() {
            @SuppressLint("SetTextI18n")
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what) {
                    case 1: //开机校验更新回传
                        break;
                    case 2:  // 反馈回传
                        break;
                    case 3:  //MQTT 收到消息回传
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
                    case 30:  //连接失败
                        Toast.makeText(MainActivity.this, "阿里云连接失败", Toast.LENGTH_SHORT).show();
                        tvStatus.setText("连接失败");
                        break;
                    case 31:   //连接成功
                        Toast.makeText(MainActivity.this, "阿里云连接成功", Toast.LENGTH_SHORT).show();
                        tvStatus.setText("已连接");
                        try {
                            client.subscribe(sub_topic, 1);
                        } catch (MqttException e) {
                            e.printStackTrace();
                        }
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
                sendControlMessage("stop");
                Toast.makeText(MainActivity.this, "发送停止指令", Toast.LENGTH_SHORT).show();
            }
        });

        btnStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendControlMessage("start");
                Toast.makeText(MainActivity.this, "发送开始指令", Toast.LENGTH_SHORT).show();
            }
        });

        btnPower.setOnCheckedChangeListener((buttonView, isChecked) -> {
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

//    private void sendControlMessage(String command) {
//        String message = "";
//
//        switch(command) {
//            case "forward":
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"btn_forward\": 1\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//            case "backward":
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"btn_backward\": 1\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//            case "left":
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"btn_left\": 1\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//            case "right":
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"btn_right\": 1\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//            case "stop":
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"btn_stop\": 1\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//            case "start":
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"btn_start\": 1\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//            case "power_on":
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"btn_power\": 1\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//            case "power_off":
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"btn_power\": 0\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//            default:
//                // 默认消息格式
//                message = "{\n" +
//                    "  \"method\": \"thing.event.property.post\",\n" +
//                    "  \"id\": \"12345\",\n" +
//                    "  \"params\": {\n" +
//                    "    \"control_command\": \"" + command + "\"\n" +
//                    "  },\n" +
//                    "  \"version\": \"1.0\"\n" +
//                    "}";
//                break;
//        }
//
//        publish_message(message);
//    }

    private void sendControlMessage(String command) {
        String message = "";

        switch(command) {
            case "forward":
                // TSL：btn_forward（int，0-10），传1符合范围
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"btn_forward\": 1\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
            case "backward":
                // TSL：btn_backward（int，0-10）
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"btn_backward\": 1\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
            case "left":
                // 若按方案A修改TSL为rw，保留此段；若按方案B，删除此段（不发送消息）
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"btn_left\": 1\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
            case "right":
                // TSL：btn_right（int，0-10）
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"btn_right\": 1\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
            case "stop":
                // TSL：btn_stop（bool，0=关，1=开），传1代表触发停止
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"btn_stop\": 1\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
            case "start":
                // TSL：btn_start（bool，0=关，1=开）
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"btn_start\": 1\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
            case "power_on":
                // TSL：btn_power（bool，1=开）
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"btn_power\": 1\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
            case "power_off":
                // TSL：btn_power（bool，0=关）
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"btn_power\": 0\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
            default:
                message = "{\n" +
                        "  \"method\": \"thing.event.property.post\",\n" +
                        "  \"id\": \"12345\",\n" +
                        "  \"params\": {\n" +
                        "    \"control_command\": \"" + command + "\"\n" +
                        "  },\n" +
                        "  \"version\": \"1.0\"\n" +
                        "}";
                break;
        }

        publish_message(message);
    }

    private void mqtt_init() {
        try {
            String clientId = "a1MoTKOqkVK.test_device1";
            Map<String, String> params = new HashMap<String, String>(16);
            params.put("productKey", productKey);
            params.put("deviceName", deviceName);
            params.put("clientId", clientId);
            String timestamp = String.valueOf(System.currentTimeMillis());
            params.put("timestamp", timestamp);
            // cn-shanghai
            String host_url = "tcp://" + productKey + ".iot-as-mqtt.cn-shanghai.aliyuncs.com:1883";
            String client_id = clientId + "|securemode=2,signmethod=hmacsha1,timestamp=" + timestamp + "|";
            String user_name = deviceName + "&" + productKey;
            String password = com.example.mqtt_app.AliyunIoTSignUtil.sign(params, deviceSecret, "hmacsha1");

            System.out.println(">>>" + host_url);
            System.out.println(">>>" + client_id);

            client = new MqttClient(host_url, client_id, new MemoryPersistence());
            options = new MqttConnectOptions();
            options.setCleanSession(false);
            options.setUserName(user_name);
            options.setPassword(password.toCharArray());
            options.setConnectionTimeout(10);
            options.setKeepAliveInterval(60);
            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    System.out.println("connectionLost----------");
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    System.out.println("deliveryComplete---------" + token.isComplete());
                }

                @Override
                public void messageArrived(String topicName, MqttMessage message)
                        throws Exception {
                    System.out.println("messageArrived----------");
                    Message msg = new Message();
                    msg.what = 3;
                    msg.obj = message.toString();
                    handler.sendMessage(msg);
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void mqtt_connect() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    if (!(client.isConnected())) {
                        client.connect(options);
                        Message msg = new Message();
                        msg.what = 31;
                        handler.sendMessage(msg);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    Message msg = new Message();
                    msg.what = 30;
                    handler.sendMessage(msg);
                }
            }
        }).start();
    }

    private void start_reconnect() {
        scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleAtFixedRate(new Runnable() {
            @Override
            public void run() {
                if (!client.isConnected()) {
                    mqtt_connect();
                }
            }
        }, 0 * 1000, 10 * 1000, TimeUnit.MILLISECONDS);
    }

    private void publish_message(String message) {
        if (client == null || !client.isConnected()) {
            Toast.makeText(MainActivity.this, "MQTT未连接", Toast.LENGTH_SHORT).show();
            return;
        }
        MqttMessage mqtt_message = new MqttMessage();
        mqtt_message.setPayload(message.getBytes());
        try {
            client.publish(pub_topic, mqtt_message);
        } catch (MqttException e) {
            e.printStackTrace();
            Toast.makeText(MainActivity.this, "消息发送失败: " + e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        udpClient.stopReceiving();
        if (scheduler != null) {
            scheduler.shutdown();
        }
    }
}