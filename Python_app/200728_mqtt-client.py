import paho.mqtt.client as mqtt
from datetime import datetime

sub = ["nhakinh/hiu2wx", "dieukhien/hiu2wx", "cmd/hiu2wx", "wifi/hiu2wx"]
mqtt_server = "emqpam.emmasoft.com.vn"
mqtt_port = 1883
user = "vuonrau"
pw = "1122332211"
print("Starting")
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    now = datetime.now()
    now = now.strftime("%H:%M:%S")
    display = "{: <15}Topic: {: <25} Payload: {}".format(now, msg.topic, msg.payload.decode('utf-8'))
    print(display)

client = mqtt.Client()
client.username_pw_set(user, pw)
client.on_connect = on_connect
client.on_message = on_message

client.connect(mqtt_server, mqtt_port, 60)
for topic in sub:
    client.subscribe(topic)
# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
