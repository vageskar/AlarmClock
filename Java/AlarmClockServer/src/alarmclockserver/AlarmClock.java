/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package alarmclockserver;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.Scanner;

/**
 *
 * @author RobinBergseth
 */
public class AlarmClock {

    private String alarm1;
    private String alarm2;
    private String alarm1Days;
    private String alarm2Days;
    

    public AlarmClock() {
        System.out.println("AlarmClockServer");
        try {
            Scanner alarms = new Scanner(new File("alarms.txt"));
            while (alarms.hasNextLine()) {
                String line = alarms.nextLine();
                if (line.contains("Alarm")) {
                    String[] parts = line.split(":");
                    System.out.println(line + "Length of parts = " + parts.length);
                    if (parts[0].equals("Alarm1")) {
                        alarm1 = parts[1];
                        alarm1Days = parts[2];
                    } else if (parts[0].equals("Alarm2")) {
                        alarm2 = parts[1];
                        alarm2Days = parts[2];
                    }
                }
            }
        } catch (FileNotFoundException e) {
            System.out.println("Create file");
            try {
                PrintWriter writer = new PrintWriter("alarms.txt", "UTF-8");
                writer.println("Alarm1:00.00:0,0,0,0,0,0,0");
                writer.println("Alarm2:00.00:0,0,0,0,0,0,0");
                writer.close();
                alarm1 = "00.00";
                alarm1Days = "0,0,0,0,0,0,0";
                alarm2 = "00.00";
                alarm2Days = "0,0,0,0,0,0,0";
            } catch (FileNotFoundException | UnsupportedEncodingException k) {
                System.out.println(" Error");
            }
        }
    }

    public void setAlarm(String time, int alarm, String days) {
        if (alarm == 1) {
            alarm1 = time;
            alarm1Days = days;
        } else if (alarm == 2) {
            alarm2 = time;
            alarm2Days = days;
        }
        try {
            PrintWriter writer = new PrintWriter("alarms.txt", "UTF-8");
            writer.println("Alarm1:" + alarm1 + ":" + getDays(1));
            writer.println("Alarm2:" + alarm2 + ":" + getDays(2));
            writer.close();
        } catch (FileNotFoundException | UnsupportedEncodingException e) {
            System.err.println(e.toString());
        }

    }

    public String getAlarm(int i) {
        if (i == 1) {
            return alarm1;
        } else if (i == 2) {
            return alarm2;
        } else {
            return null;
        }
    }

    public String getDays(int i) {
        switch (i) {
            case 1:
                return alarm1Days;
            case 2:
                return alarm2Days;
            default:
                return null;
        }
    }

}
