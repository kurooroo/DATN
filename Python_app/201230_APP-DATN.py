import kivy
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.widget import Widget
from kivy.properties import ObjectProperty
from kivy.properties import StringProperty
from kivy.clock import Clock, mainthread
from kivy.base import runTouchApp
from kivy.lang import Builder
from kivy.uix.popup import Popup 
from kivy.uix.floatlayout import FloatLayout
import webbrowser
import paho.mqtt.client as mqtt

debug = False
# MQTT protocol
topicSubscribe = ["status/datnta"]
topicPublish = "cmd/datnta" 
mqttServer = "broker.hivemq.com"
mqttPort = 1883
buttonState = [0,0,0,0]
startUp = True
flagConnected = False
_switchNumber = 0
popupWindow = True
# App GUI and logic
class PopupTurnOn(FloatLayout):
    def switch(self, switchType, switchTime):
        global  _switchNumber
        DATNApp().sendCmd(_switchNumber, switchType, switchTime)
        popupWindow.dismiss()
    def close(self):
        popupWindow.dismiss()
class PopupTurnOff(FloatLayout):
    def switch(self, switchType, switchTime):
        global  _switchNumber
        DATNApp().sendCmd(_switchNumber, switchType, switchTime)
        popupWindow.dismiss()
    def close(self):
        popupWindow.dismiss()
class MyGrid(Widget):
    button1 = ObjectProperty(None)
    button2 = ObjectProperty(None)
    button3 = ObjectProperty(None)
    button4 = ObjectProperty(None)
    configWifi = ObjectProperty(None)
    @mainthread
    def update(self, data):
        buttonUpdateState = [data[3], data[7], data[11], data[15]]
        if buttonUpdateState[0] == '1':
            self.button1.text = "Cong tac 1 Bat"
            buttonState[0] = 1
        elif buttonUpdateState[0] == '0':
            self.button1.text = "Cong tac 1 Tat"
            buttonState[0] = 0
        if buttonUpdateState[1] == '1':
            self.button2.text = "Cong tac 2 Bat"
            buttonState[1] = 1
        elif buttonUpdateState[1] == '0':
            self.button2.text = "Cong tac 2 Tat"
            buttonState[1] = 0
        if buttonUpdateState[2] == '1':
            self.button3.text = "Cong tac 3 Bat"
            buttonState[2] = 1
        elif buttonUpdateState[2] == '0':
            self.button3.text = "Cong tac 3 Tat"
            buttonState[2] = 0
        if buttonUpdateState[3] == '1':
            self.button4.text = "Cong tac 4 Bat"
            buttonState[3] = 1
        elif buttonUpdateState[3] == '0':
            self.button4.text = "Cong tac 4 Tat"
            buttonState[3] = 0
    def showPopup(self, switchNumber):
        global _switchNumber, popupWindow
        _switchNumber = int(switchNumber)
        if buttonState[_switchNumber - 1] == 0:
            show = PopupTurnOn()
            popupWindow = Popup(title = "Bat cong tac", content = show, 
                                size_hint = (None, None), size = (500,500))
        elif buttonState[_switchNumber - 1] == 1:
            show = PopupTurnOff()
            popupWindow = Popup(title = "Tat cong tac", content = show, 
                                size_hint = (None, None), size = (500,500))
        popupWindow.open()

    def buttonConfigWifi(self):
        webbrowser.open('http://192.168.4.1/wifi?#p')
        # webbrowser.open('https://kivy.org/')
    
    Clock.schedule_interval(lambda dt: DATNApp().mqttReconnect(), 30)

class DATNApp(App):
    client = mqtt.Client()
    def onConnect(self, client, userdata, flags, rc):
        if debug:
            print("Connected with result code "+str(rc))
        global flagConnected
        flagConnected = True
        for topic in topicSubscribe:
            client.subscribe(topic)
    def onDisconnect(self, client, userdata, rc):
        global flagConnected
        flagConnected = False
        if debug:
            print(flagConnected)
    def onMessage(self, client, userdata, message):
        data = message.payload.decode("utf-8")
        if debug:
            print(data)
        if data[0] == '/':
            self.root.update(data)
        # self.root._update(data[0], data[2])
    def build(self):
        try:
            self.client.onMessage = self.onMessage
            self.client.onConnect = self.onConnect
            self.client.onDisconnect = self.onDisconnect
            self.client.connect(mqttServer, mqttPort, 60)
            self.client.loop_start()
            # self.client.subscribe("status/datnta")
            if debug:
                print("Connected")
        except:
            pass
        return MyGrid()
    #switchType = 1 => Bat; 0 => tat
    def sendCmd(self, switchNumber, switchType, switchTime):
        strTemp = "/" + str(switchNumber) + "/"
        if buttonState[int(switchNumber) - 1] == 0:
            strTemp += '1'
        else:
            strTemp += '0'
        strTemp += '/' + str(switchTime)
        self.client.publish(topicPublish, strTemp)

    def mqttReconnect(self):
        if debug:
            print("clock")
        global flagConnected
        if flagConnected == False:
            if debug:
                print("try reconnect")
            try:
                # self.client.onMessage = self.onMessage
                self.client.connect(mqttServer, mqttPort, 60)
                self.client.loop_start()
                # self.client.subscribe("status/datnta")
            except:
                pass
if __name__ == "__main__":
    DATNApp().run()