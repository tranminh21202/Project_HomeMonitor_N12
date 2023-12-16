package com.example.btl_IOT.service;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import jakarta.annotation.PostConstruct;

@Service
public class MqttService {
	
    @Autowired
    private SimpMessagingTemplate simpMessagingTemplate;

    private MqttClient mqttClient;

    @PostConstruct
    public void initialize() {
        try {
            String broker = "tcp://broker.hivemq.com:1883";
            String clientId = "Le Van Dung";

            mqttClient = new MqttClient(broker, clientId);
            System.out.println("Tao mqtt client thanh cong");
            MqttConnectOptions connOpts = new MqttConnectOptions();
            connOpts.setCleanSession(true);

            mqttClient.connect(connOpts);
            
            if (mqttClient.isConnected()) {
                System.out.println("ket noi mqtt client thanh cong");
            } else {
                System.out.println("ket noi mqtt client that bai");
            }
            
            mqttClient.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {
                    handleMqttMessage(topic, new String(message.getPayload()));
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                	System.out.println("Gui message thanh cong");
                	System.out.println(".....................................................");
                }
            });

            mqttClient.subscribe("/prj/temp");
            mqttClient.subscribe("/prj/hum");
            mqttClient.subscribe("/prj/pre");
            mqttClient.subscribe("/prj/gas");
            mqttClient.subscribe("/prj/led");
            mqttClient.subscribe("/prj/buz");
            mqttClient.subscribe("/prj/smoke");
            mqttClient.subscribe("/prj/control/buz");
            mqttClient.subscribe("/prj/control/led");

        } catch (MqttException e) {
        	System.out.println("loi ket noi toi MQTT broker: " + e.getMessage());
            e.printStackTrace();
        }
    }

    public void handleMqttMessage(String topic, String message) {
        simpMessagingTemplate.convertAndSend("/topic" + topic, message);
        if(topic.equals("/prj/temp")) System.out.println("Nhiet do do duoc la: " + message);
        if(topic.equals("/prj/hum")) System.out.println("Do am do duoc la: " + message);
        if(topic.equals("/prj/gas")) System.out.println("Nong do khi gas do duoc la: " + message);
        if(topic.equals("/prj/pre")) System.out.println("Ap suat do duoc la: " + message);
    }
    
    public void publishMessage(String topic, String message) {
        try {
            System.out.println("Publish: Topic - " + topic + ", Message - " + message);

            if (mqttClient != null && mqttClient.isConnected()) {
                System.out.println("da ket noi mqtt, dang thuc hien publish");
                MqttMessage mqttMessage = new MqttMessage(message.getBytes());
                mqttClient.publish(topic, mqttMessage);
                System.out.println("publish thanh cong");
            } else {
                System.out.println("mqttClient khong ton tai hoac khong ket noi");
            }

        } catch (MqttException e) {
            System.out.println("loi khi  publish: " + e.getMessage());
            e.printStackTrace();
        }
    }


}
