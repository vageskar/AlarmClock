/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package alarmclockclient;

import java.util.Observable;
import java.util.Observer;

/**
 * Main class AlarmClockClient
 * @author RobinBergseth
 */
public class MainAlarmClock {

    Client client;
    AlarmClockGUI gui;
    AlarmData ad;

    public MainAlarmClock() {
        ad = new AlarmData();
        //(new Thread(client = new Client(ad))).start();
        gui = new AlarmClockGUI(ad);
        gui.setVisible(true);
        ad.addObserver(gui);
    }

    public static void main(String args[]) {
        new MainAlarmClock();
    }

}
