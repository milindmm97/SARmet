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

<<<<<<< HEAD
public class GradientTranslucentWindowDemo extends JFrame {
    public GradientTranslucentWindowDemo() {
        super("GradientTranslucentWindow");

        setBackground(new Color(0,0,0,0));
        setSize(new Dimension(640, 480));				//modify the size
       setLocationRelativeTo(null);				// set location to bottom

        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE); // replace with timer 
		
		setUndecorated(true);

        JPanel panel = new JPanel() {
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
        setContentPane(panel);
        setLayout(new GridBagLayout());
		JLabel label1 = new JLabel("Test");
		label1.setFont(new Font("Serif", Font.BOLD, 48));
		label1.setForeground(Color.white);
        add(label1);
		
		
    }
=======

public class host_string {






>>>>>>> b9403744d26f82ef4e90d8f9ccf6195e43f156f1

    public static void main(String[] args) {

JFrame frame =new JFrame();
frame.setSize(600,400);
frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

JPanel panel=new JPanel() ;
panel.setBackground(Color.YELLOW);
//frame.setVisible(true);




        System.out.println("Server start");
        System.out.println("Runtime Java: " 
                + System.getProperty("Java.runtime.version"));
        
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
				frame.setVisible(true);

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
