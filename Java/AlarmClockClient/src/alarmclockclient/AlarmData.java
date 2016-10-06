/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package alarmclockclient;

import java.util.Observable;

/**
 *
 * @author RobinBergseth
 */
public class AlarmData extends Observable {

    String alarm1 = "00:00";
    String alarm2 = "00:00";
    String alarm1Days = "0,0,0,0,0,0,0";
    String alarm2Days = "0,0,0,0,0,0,0";

    public void setAlarm(String[] values) {
        for (String s : values) {
            String[] parts = s.split(":");
            if (parts.length > 1) {
                if (parts[0].equals("Alarm1")) {
                    alarm1 = parts[1];
                    alarm1Days = parts[2];
                } else if (parts[0].equals("Alarm2")) {
                    alarm2 = parts[1];
                    alarm2Days = parts[2];
                }
            }
        }
        setChanged();
        notifyObservers();
    }

    public String[] getAlarms() {
        String[] retString = {alarm1, alarm2};
        return retString;
    }
    public String[] getDays(){
        String[] setString = {alarm1Days, alarm2Days};
        return setString;
    }
}


