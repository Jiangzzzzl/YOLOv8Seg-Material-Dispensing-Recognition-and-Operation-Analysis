import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.Scanner;

public class SimpleTCPClient {
    public static void main(String[] args) {
        String serverIp = "192.168.3.16";  // 使用Android设备的新IP地址
        int serverPort = 8086;  // 使用与服务器相同的端口号
        Socket socket = null;
        
        try {
            System.out.println("连接到服务器: " + serverIp + ":" + serverPort);
            socket = new Socket(serverIp, serverPort);
            System.out.println("连接成功!");
            
            // 创建输入输出流
            DataOutputStream outputStream = new DataOutputStream(socket.getOutputStream());
            BufferedReader inputStream = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            
            // 启动接收消息线程
            Thread receiveThread = new Thread(() -> {
                try {
                    String message;
                    while ((message = inputStream.readLine()) != null) {
                        System.out.println("【收到服务器消息】: " + message);
                    }
                } catch (Exception e) {
                    System.err.println("接收消息时出错: " + e.getMessage());
                }
            });
            receiveThread.start();
            
            // 等待欢迎消息
            Thread.sleep(1000);
            
            // 交互式发送消息
            Scanner scanner = new Scanner(System.in);
            System.out.println("请输入要发送给服务器的消息 (输入'quit'退出):");
            
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
                    System.out.println("消息已发送: " + input);
                }
            }
            
            // 关闭连接
            socket.close();
            System.out.println("连接已关闭");
            
        } catch (Exception e) {
            System.err.println("客户端出错: " + e.getMessage());
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