package com.example.riya.notifapp;


import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    EditText editTextAddress, editTextPort, editTextMsg;
    Button buttonConnect, buttonDisconnect, buttonSend;
    TextView textViewState, textViewRx;

    ClientHandler clientHandler;
    ClientThread clientThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        editTextAddress = (EditText) findViewById(R.id.address);
        editTextPort = (EditText) findViewById(R.id.port);
        editTextMsg = (EditText) findViewById(R.id.msgtosend);
        buttonConnect = (Button) findViewById(R.id.connect);
        buttonDisconnect = (Button) findViewById(R.id.disconnect);
        buttonSend = (Button)findViewById(R.id.send);
        textViewState = (TextView)findViewById(R.id.state);
        textViewRx = (TextView)findViewById(R.id.received);

        buttonDisconnect.setEnabled(false);
        buttonSend.setEnabled(false);

        buttonConnect.setOnClickListener(buttonConnectOnClickListener);
        buttonDisconnect.setOnClickListener(buttonDisConnectOnClickListener);
        buttonSend.setOnClickListener(buttonSendOnClickListener);

        clientHandler = new ClientHandler(this);
    }

    View.OnClickListener buttonConnectOnClickListener =
            new View.OnClickListener() {

                @Override
                public void onClick(View arg0) {

                    clientThread = new ClientThread(
                            editTextAddress.getText().toString(),
                            Integer.parseInt(editTextPort.getText().toString()),
                            clientHandler);
                    clientThread.start();

                    buttonConnect.setEnabled(false);
                    buttonDisconnect.setEnabled(true);
                    buttonSend.setEnabled(true);
                }
            };

    View.OnClickListener buttonDisConnectOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            if(clientThread != null){
                clientThread.setRunning(false);
            }

        }
    };

    View.OnClickListener buttonSendOnClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            if(clientThread != null){
                String msgToSend = editTextMsg.getText().toString();
                clientThread.txMsg(msgToSend);
            }
        }
    };

    private void updateState(String state){
        textViewState.setText(state);
    }

    private void updateRxMsg(String rxmsg){
        textViewRx.append(rxmsg + "\n");
    }

    private void clientEnd(){
        clientThread = null;
        textViewState.setText("clientEnd()");
        buttonConnect.setEnabled(true);
        buttonDisconnect.setEnabled(false);
        buttonSend.setEnabled(false);

    }

    public static class ClientHandler extends Handler {
        public static final int UPDATE_STATE = 0;
        public static final int UPDATE_MSG = 1;
        public static final int UPDATE_END = 2;
        private MainActivity parent;

        public ClientHandler(MainActivity parent) {
            super();
            this.parent = parent;
        }

        @Override
        public void handleMessage(Message msg) {

            switch (msg.what){
                case UPDATE_STATE:
                    parent.updateState((String)msg.obj);
                    break;
                case UPDATE_MSG:
                    parent.updateRxMsg((String)msg.obj);
                    break;
                case UPDATE_END:
                    parent.clientEnd();
                    break;
                default:
                    super.handleMessage(msg);
            }

        }

    }
}