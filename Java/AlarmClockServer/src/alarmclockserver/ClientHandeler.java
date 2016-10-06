/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package alarmclockserver;

import java.io.DataInputStream;
import java.io.PrintStream;
import java.net.Socket;
import java.util.Scanner;

/**
 *
 * @author RobinBergseth
 */
class ClientHandeler extends Thread {

    private Socket conn;
    private AlarmClock alarm;

    public ClientHandeler(Socket conn) {
        this.conn = conn;
        System.out.println("Create AlarmClock");
        alarm = new AlarmClock();
    }

    public void run() {
        String line = "";
        try {
            DataInputStream in = new DataInputStream(conn.getInputStream());
            PrintStream out = new PrintStream(conn.getOutputStream());
            boolean exit = false;
            while ((line = in.readLine()) != null && !exit) {
                String cmdWord = null;
                String request = null;
                String data = null;
                String days = null;
                String outPrint = null;
                Scanner scan = new Scanner(line);
                if (scan.hasNext()) {
                    cmdWord = scan.next().toLowerCase();
                    if (scan.hasNext()) {
                        request = scan.next().toLowerCase();
                        if (scan.hasNext()) {
                            data = scan.next().toLowerCase();
                            if(scan.hasNext()){
                                days = scan.next().toLowerCase();
                            }
                        }
                    }
                }
                if (cmdWord == null) {
                    cmdWord = "";
                }
                if (request == null) {
                    request = "";
                }
                switch (cmdWord) {
                    case "get":
                        switch (request) {
                            case "alarm1":
                                outPrint = "Alarm1:" + alarm.getAlarm(1) + ":" + alarm.getDays(1);
                                break;
                            case "alarm2":
                                outPrint = "Alarm2:" + alarm.getAlarm(2) + ":" + alarm.getDays(2);
                                break;
                            case "all":
                                outPrint = "Alarm1:" + alarm.getAlarm(1) + ":" + alarm.getDays(1)
                                        + ";Alarm2:" + alarm.getAlarm(2) + ":" + alarm.getDays(2);
                                break;
                            default:
                                outPrint = returnBadRequest();
                                break;
                        }
                        break;
                    case "set":
                        switch (request) {
                            case "alarm1":
                                if (data != null && days != null) {
                                    alarm.setAlarm(data, 1, days);
                                    outPrint = "Alarm OK";
                                } else {
                                    outPrint = returnBadRequest();
                                }
                                break;
                            case "alarm2":
                                if (data != null && days != null) {
                                    alarm.setAlarm(data, 2, days);
                                    outPrint = "Alarm OK";
                                } else {
                                    outPrint = returnBadRequest();
                                }
                                break;
                            default:
                                outPrint = returnBadRequest();
                                break;
                        }
                        break;
                    case "quit":
                        exit = true;
                        break;
                    case "shutdown":
                        conn.close();
                        System.exit(1);
                        break;
                    default:
                        outPrint = returnBadRequest();
                        break;

                }
                if (outPrint != null) {
                    out.println(outPrint);
                }
            }
            conn.close();
        } catch (Exception e) {
            System.err.println(e.toString());
        }
    }

    private String returnBadRequest() {
        return "BadReq";
    }
}
