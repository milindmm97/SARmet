import java.awt.*;
import javax.swing.*;
//import javax.util.Timer;
import static java.awt.GraphicsDevice.WindowTranslucency.*;

public class GradientTranslucentWindowDemo extends JFrame {
    public GradientTranslucentWindowDemo() {
        super("GradientTranslucentWindow");

        setBackground(new Color(0,0,0,0));
        setSize(new Dimension(320, 240));				//modify the size
        setLocation(60,120);					// set location to bottom

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
        add(label1);
		
		
    }

    public static void main(String[] args) {
        // Determine what the GraphicsDevice can support.
        GraphicsEnvironment ge = 
            GraphicsEnvironment.getLocalGraphicsEnvironment();
        GraphicsDevice gd = ge.getDefaultScreenDevice();
        boolean isPerPixelTranslucencySupported = 
            gd.isWindowTranslucencySupported(PERPIXEL_TRANSLUCENT);

        //If translucent windows aren't supported, exit.
        if (!isPerPixelTranslucencySupported) {
            System.out.println(
                "Per-pixel translucency is not supported");
                System.exit(0);
        }

        JFrame.setDefaultLookAndFeelDecorated(true);

        // Create the GUI on the event-dispatching thread
        SwingUtilities.invokeLater(new Runnable() {
            @Override
			//Timer timer = new Timer();
			//TimerTask task = new TimerTask(){	
            public void run() 
			{
                GradientTranslucentWindowDemo gtw = new
                    GradientTranslucentWindowDemo();

                // Display the window.
                gtw.setVisible(true);
				try{Thread.sleep(3000);}
				catch(Exception e){
					//System.println("message");
				}
				
			
				gtw.dispose();
					
				//setTimeout(function(){ gtw.dispose() }, 1000);
				

				
            }
        });
    }
}