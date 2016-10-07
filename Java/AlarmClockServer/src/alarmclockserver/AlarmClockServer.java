
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package alarmclockserver;

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * Main class AlarmClockServer
 * @author RobinBergseth
 */
public class AlarmClockServer{

    private ServerSocket server;
    private Socket skt;
    private int port = 2609;
            
    public AlarmClockServer(){
        try {
            server = new ServerSocket(port);
            System.out.println(InetAddress.getLocalHost().getCanonicalHostName());
            while (true) {
                skt = server.accept();                
                (new Thread(new ClientHandeler(skt))).start();
            }
        } catch (IOException e) {
            System.err.println(e.toString());
        }
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        new AlarmClockServer();
    }
}
