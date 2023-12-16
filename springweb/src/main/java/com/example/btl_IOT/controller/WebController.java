package com.example.btl_IOT.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.stereotype.Controller;
import org.springframework.web.socket.config.annotation.EnableWebSocket;
import org.springframework.web.socket.config.annotation.EnableWebSocketMessageBroker;

import com.example.btl_IOT.service.MqttService;

@Controller
@EnableWebSocketMessageBroker
@EnableWebSocket
public class WebController {
	@Autowired
	MqttService mqttService;
 
    @MessageMapping("/control/led")
    public String controlLed(String command) {
    	System.out.print("dieu khien den: ");
    	String s = "";
        if (command.equals("on")) {
            mqttService.publishMessage("/prj/control/led", "on");
            s = "den da duoc bat";
            System.out.println(s);
        }else{	
            mqttService.publishMessage("/prj/control/led", "off");
            s = "den da duoc tat";
            System.out.println(s);
        }
        return s;
    }

    @MessageMapping("/control/buz")
    public String controlBuz(String command) {
    	System.out.print("dieu khien coi: ");
    	String s = "";
        if (command.equals("on")) {
            mqttService.publishMessage("/prj/control/buz", "on");
            s = "coi da duoc bat";
            System.out.println(s);
        }else {
            mqttService.publishMessage("/prj/control/buz", "off");
            s = "coi da duoc tat";
            System.out.println(s);
        }
        return s;
    }
}
