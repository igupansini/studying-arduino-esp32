package com.impansini.poc_mqtt;

import org.eclipse.paho.client.mqttv3.MqttException;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/command")
public class CommandController {

    private final MqttService mqtt;
    private final String topicCommand;

    public CommandController(MqttService mqtt, @Value("${mqtt.topic-command}") String topicCommand) {
        this.mqtt = mqtt;
        this.topicCommand = topicCommand;
    }

    @PostMapping("/{state}")
    public ResponseEntity<String> send(@PathVariable String state) throws MqttException {
        mqtt.publish(topicCommand, state);
        return ResponseEntity.ok("Command published to " + topicCommand + ": " + state);
    }
}
