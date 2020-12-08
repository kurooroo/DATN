import kivy
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.widget import Widget
from kivy.properties import ObjectProperty
from kivy.properties import StringProperty
from kivy.clock import Clock, mainthread
import paho.mqtt.client as mqtt

# MQTT protocol
topicSubscribe = ["status/datnta"]
topicPublish = "cmd/datnta" 
mqtt_server = "broker.hivemq.com"
mqtt_port = 1883
touchFlag = False
# user = ""
# pw = ""
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    for topic in topicSubscribe:
        client.subscribe(topic)

# client.username_pw_set(user, pw)
# client = mqtt.Client()
# App GUI and logic
class MyGrid(Widget):
    button = ObjectProperty(None)

    @mainthread
    def btn(self, client, bt_number):
        client.publish(topicPublish, "5")
    def update(self, data):
        self.button.text = str(data)

class DATNApp(App):
    client = mqtt.Client()
    def on_message(self, client, userdata, message):
        data = message.payload.decode("utf-8")
        self.root.update(data)
    def build(self):
        self.client.on_message = self.on_message
        self.client.connect(mqtt_server, mqtt_port, 60)
        self.client.loop_start()
        self.client.subscribe("status/datnta")
        return MyGrid()
    
if __name__ == "__main__":
    DATNApp().run()


