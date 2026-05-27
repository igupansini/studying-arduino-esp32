package com.impansini.poc_mqtt;

import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

@Service
public class MqttService implements MqttCallback {

    private static final Logger log = LoggerFactory.getLogger(MqttService.class);

    private final String brokerUrl;
    private final String clientId;
    private final String topicStatus;

    private MqttClient client;

    public MqttService(
            @Value("${mqtt.broker-url}") String brokerUrl,
            @Value("${mqtt.client-id}") String clientId,
            @Value("${mqtt.topic-status}") String topicStatus) {
        this.brokerUrl = brokerUrl;
        this.clientId = clientId;
        this.topicStatus = topicStatus;
    }

    @PostConstruct
    public void connect() throws MqttException {
        client = new MqttClient(brokerUrl, clientId, new MemoryPersistence());

        MqttConnectOptions opts = new MqttConnectOptions();
        opts.setAutomaticReconnect(true);
        opts.setCleanSession(true);
        opts.setConnectionTimeout(10);

        client.setCallback(this);
        client.connect(opts);
        client.subscribe(topicStatus);

        log.info("MQTT connected to {} | clientId={} | subscribed to {}", brokerUrl, clientId, topicStatus);
    }

    public void publish(String topic, String payload) throws MqttException {
        client.publish(topic, new MqttMessage(payload.getBytes()));
        log.info("[TX] {} :: {}", topic, payload);
    }

    @PreDestroy
    public void disconnect() throws MqttException {
        if (client != null && client.isConnected()) {
            client.disconnect();
        }
    }

    @Override
    public void connectionLost(Throwable cause) {
        log.warn("MQTT connection lost", cause);
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) {
        log.info("[RX] {} :: {}", topic, new String(message.getPayload()));
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
    }
}
