import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;

public class MessageTestClient {
    public static void main(String[] args) {
        String serverIp = "192.168.3.16";  // 使用Android设备的IP地址
        int serverPort = 8086;  // 使用与服务器相同的端口号
        
        try {
            System.out.println("【测试客户端】连接到服务器: " + serverIp + ":" + serverPort);
            Socket socket = new Socket(serverIp, serverPort);
            System.out.println("【测试客户端】连接成功!");
            
            // 创建输入输出流
            DataOutputStream outputStream = new DataOutputStream(socket.getOutputStream());
            BufferedReader inputStream = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            
            // 启动接收消息线程
            Thread receiveThread = new Thread(() -> {
                try {
                    System.out.println("【测试客户端】启动接收线程");
                    String message;
                    while ((message = inputStream.readLine()) != null) {
                        System.out.println("【测试客户端】<<< 收到消息: " + message);
                    }
                    System.out.println("【测试客户端】接收线程结束");
                } catch (Exception e) {
                    System.err.println("【测试客户端】接收消息时出错: " + e.getMessage());
                }
            });
            receiveThread.start();
            
            // 等待欢迎消息
            System.out.println("【测试客户端】等待欢迎消息...");
            Thread.sleep(2000);
            
            // 发送一个简单的测试消息
            System.out.println("【测试客户端】>>> 发送测试消息");
            String testMessage = "测试消息\n";
            outputStream.write(testMessage.getBytes("UTF-8"));
            outputStream.flush();
            System.out.println("【测试客户端】测试消息已发送");
            
            // 等待一段时间看是否有响应
            System.out.println("【测试客户端】等待服务器响应...");
            Thread.sleep(3000);
            
            // 发送按钮点击模拟消息
            System.out.println("【测试客户端】>>> 发送按钮点击模拟消息");
            String buttonMessage = "{\n" +
                "  \"method\": \"thing.event.property.post\",\n" +
                "  \"id\": \"99999\",\n" +
                "  \"params\": {\n" +
                "    \"btn_test\": 1\n" +
                "  },\n" +
                "  \"version\": \"1.0\"\n" +
                "}\n";
            outputStream.write(buttonMessage.getBytes("UTF-8"));
            outputStream.flush();
            System.out.println("【测试客户端】按钮点击模拟消息已发送");
            
            // 再等待一段时间
            Thread.sleep(3000);
            
            // 关闭连接
            System.out.println("【测试客户端】关闭连接");
            socket.close();
            System.out.println("【测试客户端】连接已关闭");
            
        } catch (Exception e) {
            System.err.println("【测试客户端】出错: " + e.getMessage());
            e.printStackTrace();
        }
    }
}