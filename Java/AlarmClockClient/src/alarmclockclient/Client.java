/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package alarmclockclient;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.Scanner;

/**
 *
 * @author RobinBergseth
 */
public class Client implements Runnable {

    private Socket s;
    private String host = "localhost";//"158.38.198.28";
    private int port = 2609;
    private PrintWriter s_out;
    private DataInputStream s_in;
    private String message;
    private AlarmData alarmData;

    public Client(AlarmData ad) {
        alarmData = ad;

    }

    @Override
    public void run() {
        boolean run = true;
        s = new Socket();
        try {
            s.connect(new InetSocketAddress(host, port));
            s_out = new PrintWriter(s.getOutputStream(), true);
            s_in = new DataInputStream(s.getInputStream());
        } catch (IOException e) {
            System.out.println(e.toString());
            System.exit(1);
        }
        s_out.println("Get all");
        while (run) {
            // Check if the connection stil is valid
            if (s.isClosed()) {
                run = false;
                break;
            }
            String in;
            String cmd = null;
            String data = null;
            try {
                if ((in = s_in.readLine()) != null) {
                    if (in.equals("BadReq")) {
                        System.out.println("Bad request");
                    } else if (in.equals("Alarm OK")) {
                        s_out.println("Get all");
                    } else {
                        alarmData.setAlarm(in.split(";"));
                    }
                }
            } catch (IOException e) {
                System.out.println(e.toString());
                System.exit(1);
            }

        }
        // Close the in and out stream and the socket connection
        try {
            s_out.close();
            s_in.close();
            s.close();
        } catch (IOException e) {
            System.out.println(e.toString());
            System.exit(1);
        }
    }

    public void send(String s) {
        s_out.println(s);
    }

}
