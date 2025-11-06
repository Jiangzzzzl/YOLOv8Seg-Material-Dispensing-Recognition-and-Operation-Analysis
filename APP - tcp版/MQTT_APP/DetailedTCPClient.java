import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.Scanner;

public class DetailedTCPClient {
    public static void main(String[] args) {
        String serverIp = "192.168.3.16";  // 使用Android设备的新IP地址
        int serverPort = 8086;  // 使用与服务器相同的端口号
        Socket socket = null;
        
        try {
            System.out.println("【TCP客户端】尝试连接到服务器: " + serverIp + ":" + serverPort);
            socket = new Socket(serverIp, serverPort);
            System.out.println("【TCP客户端】连接成功!");
            
            // 创建输入输出流
            DataOutputStream outputStream = new DataOutputStream(socket.getOutputStream());
            BufferedReader inputStream = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            
            // 启动接收消息线程
            Thread receiveThread = new Thread(() -> {
                try {
                    System.out.println("【TCP客户端】启动接收消息线程");
                    String message;
                    while ((message = inputStream.readLine()) != null) {
                        System.out.println("【TCP客户端】<<< 收到服务器消息: " + message);
                    }
                    System.out.println("【TCP客户端】接收线程结束");
                } catch (Exception e) {
                    System.err.println("【TCP客户端】接收消息时出错: " + e.getMessage());
                    e.printStackTrace();
                }
            });
            receiveThread.start();
            
            // 等待欢迎消息
            System.out.println("【TCP客户端】等待服务器欢迎消息...");
            Thread.sleep(2000);
            
            // 发送测试消息
            System.out.println("【TCP客户端】>>> 发送测试消息");
            String testMessage = "测试消息内容\n";
            outputStream.write(testMessage.getBytes("UTF-8"));
            outputStream.flush();
            System.out.println("【TCP客户端】测试消息已发送: " + testMessage.trim());
            
            // 等待一段时间看是否有响应
            System.out.println("【TCP客户端】等待服务器响应...");
            Thread.sleep(3000);
            
            // 交互式发送消息
            Scanner scanner = new Scanner(System.in);
            System.out.println("【TCP客户端】请输入要发送给服务器的消息 (输入'quit'退出):");
            
            while (true) {
                System.out.print(">>> ");
                String input = scanner.nextLine();
                
                if ("quit".equals(input)) {
                    break;
                }
                
                if (!input.isEmpty()) {
                    // 添加换行符
                    outputStream.write((input + "\n").getBytes("UTF-8"));
                    outputStream.flush();
                    System.out.println("【TCP客户端】消息已发送: " + input);
                }
            }
            
            // 关闭连接
            System.out.println("【TCP客户端】关闭连接");
            socket.close();
            System.out.println("【TCP客户端】连接已关闭");
            
        } catch (Exception e) {
            System.err.println("【TCP客户端】客户端出错: " + e.getMessage());
            e.printStackTrace();
        } finally {
            if (socket != null && !socket.isClosed()) {
                try {
                    socket.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }
}