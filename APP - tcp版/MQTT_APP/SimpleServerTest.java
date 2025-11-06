import java.io.DataOutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;

public class SimpleServerTest {
    public static void main(String[] args) {
        int port = 8086;
        
        try {
            System.out.println("【测试服务器】启动服务器，端口: " + port);
            ServerSocket serverSocket = new ServerSocket(port);
            System.out.println("【测试服务器】服务器启动成功，等待客户端连接...");
            
            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("【测试服务器】客户端连接成功: " + clientSocket.getInetAddress());
                
                // 创建输入输出流
                DataOutputStream outputStream = new DataOutputStream(clientSocket.getOutputStream());
                BufferedReader inputStream = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                
                // 发送欢迎消息
                String welcomeMessage = "欢迎连接到测试服务器!\n";
                outputStream.write(welcomeMessage.getBytes("UTF-8"));
                outputStream.flush();
                System.out.println("【测试服务器】欢迎消息发送成功: " + welcomeMessage.trim());
                
                // 启动接收消息线程
                Thread receiveThread = new Thread(() -> {
                    try {
                        String message;
                        while ((message = inputStream.readLine()) != null) {
                            System.out.println("【测试服务器】收到客户端消息: " + message);
                            
                            // 回复消息
                            String replyMessage = "服务器收到消息: " + message + "\n";
                            outputStream.write(replyMessage.getBytes("UTF-8"));
                            outputStream.flush();
                            System.out.println("【测试服务器】回复消息发送成功: " + replyMessage.trim());
                        }
                    } catch (Exception e) {
                        System.err.println("【测试服务器】接收消息时出错: " + e.getMessage());
                    }
                });
                receiveThread.start();
            }
        } catch (Exception e) {
            System.err.println("【测试服务器】出错: " + e.getMessage());
            e.printStackTrace();
        }
    }
}