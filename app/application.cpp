#include <user_config.h>
#include <SmingCore/SmingCore.h>



// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "Maker Ha Noi" // Put you SSID and Password here
	#define WIFI_PWD "makerhanoi123"
#endif

// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);

Timer procTimer;
int light_value =0;
// MQTT client
// For quickly check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
MqttClient mqtt("test.mosquitto.org", 1883, onMessageReceived);
HttpClient thingSpeak;

void onDataSent(HttpClient& client, bool successful)
{
	if (successful)
		Serial.println("Success sent");
	else
		Serial.println("Failed");

	String response = client.getResponseString();
	Serial.println("Server response: '" + response + "'");
	if (response.length() > 0)
	{
		int intVal = response.toInt();

		if (intVal == 0)
			Serial.println("Sensor value wasn't accepted. May be we need to wait a little?");
	}
}
void sendData()
{
	if (thingSpeak.isProcessing()) return; // We need to wait while request processing was completed

	// Read our sensor value :)
	//sensorValue = dht.readHumidity();
	thingSpeak.downloadString("http://api.thingspeak.com/update?key=69TT2XX59AN4FO1F&field1=" + String(light_value), onDataSent);
}
// Publish our message
void publishMessage()
{
	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	Serial.println("Let's publish message now!");
	light_value = system_adc_read();
	Serial.print("da doc duoc "+String(light_value)+"---********************-----");
	mqtt.publish("haibn.uet", String(light_value)); // or publishWithQoS
	sendData();
}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Prettify alignment for printing
	Serial.println(message);

}

// Run MQTT client
void startMqttClient()
{
	if(!mqtt.setWill("last/will","The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}
	mqtt.connect("esp8266-sender");
	mqtt.subscribe("haibn.uet");
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	// Run MQTT client
	startMqttClient();

	// Start publishing loop
	procTimer.initializeMs(20 * 1000, publishMessage).start(); // every 20 seconds
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}



void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
}
