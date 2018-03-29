import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import javax.swing.*;
import java.awt.*;
import static java.awt.GraphicsDevice.WindowTranslucency.*;


public class host_string {







    public static void main(String[] args) {

JFrame frame =new JFrame();
frame.setSize(600,400);
frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
panel.setBackground(new Color(0,0,0,0));
JPanel panel=new JPanel() {
            @Override
            protected void paintComponent(Graphics g) {
                if (g instanceof Graphics2D) {
                    final int R = 250;
                    final int G = 250;
                    final int B = 250;

                    Paint p =
                        new GradientPaint(0.0f, 0.0f, new Color(R, G, B, 0),
                            0.0f, getHeight(), new Color(R, G, B, 255), true);
                    Graphics2D g2d = (Graphics2D)g;
                    g2d.setPaint(p);
                    g2d.fillRect(0, 0, getWidth(), getHeight());
                }
            }
        };

frame.setVisible(true);
try{Thread.sleep(3000);}
				catch(Exception e){
					//System.println("message");
				}
				
			
				frame.dispose();




        System.out.println("Server start");
        System.out.println("Runtime Java: " 
                + System.getProperty("java.runtime.version"));
        
        ServerSocket serverSocket = null;
        Socket clientSocket = null;
        BufferedReader bufferedReader = null;
        PrintWriter printWriter = null;
        
        try {
            serverSocket = new ServerSocket(8090);
            System.out.println("Server port: " 
                    + serverSocket.getLocalPort());
            
            clientSocket = serverSocket.accept();
            
            //Client connected

            InputStream inputStream = clientSocket.getInputStream();
            InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
            bufferedReader = new BufferedReader(inputStreamReader);
            
            OutputStream outputStream = clientSocket.getOutputStream();
            printWriter = new PrintWriter(outputStream, true);

            String line;
            while((line = bufferedReader.readLine()) != null){
                System.out.println(line);
                JLabel label = new JLabel(line);
               
				panel.add(label);
				frame.add(panel);

                printWriter.println(line);  //echo back to sender
            };
            
        
} catch (IOException ex) {
            System.err.println(ex.toString());
        }finally{
            
            if(printWriter != null){
                printWriter.close();
                System.out.println("printWriter closed");
            }
            
            if(bufferedReader != null){
                try {
                    bufferedReader.close();
                    System.out.println("bufferedReader closed");
                } catch (IOException ex) {
                    System.out.println(ex.toString());
                }
            }
            
            if(clientSocket != null){
                try {
                    clientSocket.close();
                    System.out.println("clientSocket closed");
                } catch (IOException ex) {
                    System.out.println(ex.toString());
                }
            }
            
            if(serverSocket != null){
                try {
                    serverSocket.close();
                    System.out.println("serverSocket closed");
                } catch (IOException ex) {
                    System.out.println(ex.toString());
                }
            }
            
        }
        
    }
    
}
