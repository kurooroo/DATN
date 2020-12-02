import kivy
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.widget import Widget
from kivy.properties import ObjectProperty
from kivy.properties import StringProperty
import paho.mqtt.client as mqtt

# MQTT protocol
topicSubscribe = ["status/datnta"]
topicPublish = "cmd/datnta" 
mqtt_server = "broker.hivemq.com"
mqtt_port = 1883
# user = ""
# pw = ""
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    for topic in topicSubscribe:
        client.subscribe(topic)

def on_message(client, userdata, msg):
    print(msg)
    if bt1_state == 0:
        bt1_state = 1
    else:
        bt1_state = 0

client = mqtt.Client()
# client.username_pw_set(user, pw)
client.on_connect = on_connect
client.on_message = on_message

client.connect(mqtt_server, mqtt_port, 60)


# App GUI and logic
class MyGrid(Widget):
    bt1_state = StringProperty()
    bt1_state = 0
    def btn(self, bt_number):
        client.publish(topicPublish, bt_number)

class DATNApp(App):
    def build(self):
        return MyGrid()

if __name__ == "__main__":
    DATNApp().run()

client.loop_forever()
