/*
 Name:		NexusConnectToWiFi.ino
 Created:	1/24/2023 2:02:20 PM
 Author:	Luke Renton
*/

#include <Arduino.h>
#include <WiFi.h>
#include "WebServer.h"
#include <string>


#define WIFI_TIMEOUT_MS 20000
#define LED_STATUS 48
#define LED_WiFi 2

WebServer wifi_server(80);
WebServer web_server(80);
IPAddress Nexus_IP_Address;
String other_networks = "";
const char* WIFI_NETWORK = "";
const char* WIFI_PASSWORD = "";

uint8_t LED1pin = LED_STATUS;
bool LED1status = LOW;

uint8_t LED2pin = LED_WiFi;
bool LED2status = LOW;
// the setup function runs once when you press reset or power the board
using namespace std;

String Format_CSS() {
	String CSS = "<!DOCTYPE html> <html>\n";
	CSS += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
	CSS += "<title>LED Control</title>\n";
	CSS += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
	CSS += "body{margin-top: 50px;background-color : #000000} h1 {color: white;margin: 50px auto 30px;} h3 {color: white;margin-bottom: 50px;}\n";
	CSS += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
	CSS += ".button-on {background-color: #3498db;}\n";
	CSS += ".button-on:active {background-color: #2980b9;}\n";
	CSS += ".button-off {background-color: #34495e;}\n";
	CSS += ".button-off:active {background-color: #2c3e50;}\n";
	CSS += ".button-label {display: inline- block; padding: 10px 20px;  text - align: center; text - decoration: none; color: #ffffff; background - color: #3498db;border - radius: 6px;outline: none;}\n";
	CSS += "p {font-size: 14px;color: white;margin-bottom: 10px;}\n";
	CSS += "</style>\n";
	CSS += "</head>\n";
	return CSS;
}

String SendHTML_login() {

	String HTML = Format_CSS();
	HTML += "<body>\n <h3> Please enter the name and password for the network below </h3>";
	HTML += "<form action=\"/Joined\">";
	HTML += "<label for = \"network_name\"> <font color = \"white\">Network Name: </font> </label>";
	HTML += "<input type=\"text\" id=\"Network_Name\" name=\"Network_Name\" value=\"\"><br><br>";
	HTML += "<label for =\"Password\"> <font color = \"white\">Password: </font> </label>";
	HTML += "<input type=\"Password\" id=\"Password\" name=\"Password\" value=\"\"><br><br>";
	HTML += "<input type=\"submit\" value=\"Submit\"></form>";
	HTML += "<br><br> <h3> Available networks: </h3>";
	HTML += other_networks;
	HTML += "</body>\n</html>\n";

	/*HTML += "<form action = \"/get\"> <font color = \"white\">Network Name: </font><input type=\"input\" name=\"Network\"></form><br>";
	HTML += "<form action=\"/get\"> <font color = \"white\">Password: </font><input type=\"password\" name=\"Password\"></form><br>";
	HTML += "<a class=\"button-label\" href=\"/Joined?Network_Name=IDX&Password=123\">Join network</a>";*/
	return HTML;

}

String SendHTML_main(uint8_t led1stat, uint8_t led2stat) {
	String HTML = Format_CSS();
	//For what ever reason raw string types R"()" don't work so have to append line by line
	HTML += "<body>\n";
	HTML += "<h1>ESP32 Web Server</h1>\n";
	HTML += "<h3>Using Access Point(AP) Mode</h3>\n";

	if (led1stat)
	{
		HTML += "<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";
	}
	else
	{
		HTML += "<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";
	}

	if (led2stat)
	{
		HTML += "<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";
	}
	else
	{
		HTML += "<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";
	}

	HTML += "<a href=\"/Join\" class=\"button-label\"> Join a network </a>";
	HTML += "</body>\n</html>\n";
	return HTML;
}

String get_encryption_type(wifi_auth_mode_t encryptionType) {
	switch (encryptionType) {
	case (WIFI_AUTH_OPEN):
		return "Open";
	case (WIFI_AUTH_WEP):
		return "WEP";
	case (WIFI_AUTH_WPA_PSK):
		return "WPA_PSK";
	case (WIFI_AUTH_WPA2_PSK):
		return "WPA2_PSK";
	case (WIFI_AUTH_WPA_WPA2_PSK):
		return "WPA_WPA2_PSK";
	case (WIFI_AUTH_WPA2_ENTERPRISE):
		return "WPA2_ENTERPRISE";
	}
}

string get_WiFi_list() {
	int networks, selected_networks;
	string network_info = "";

	networks = WiFi.scanNetworks();

	for (int i = 0; i < networks; ++i) {
		// Print SSID, RSSI and WiFi Encryption for each network found
		network_info += "<p> [" + to_string(i + 1) + "]	" + WiFi.SSID(i).c_str() +
			" (" + to_string(WiFi.RSSI(i)) + " db) [" + get_encryption_type(WiFi.encryptionType(i)).c_str() +
			"] </p>\n";
		//<a href=\"/Join\">Join network</a>
		//"<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";
		//client.print("Click <a href=\"/H\">here</a> to turn the red LED on.<br>");
	}

	return network_info;
}


void connect_to_WiFi() {
	unsigned long startAttemptTime = millis();
	// WiFi.scanNetworks will return the number of networks found
	WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

	Serial.print(">>>>>> Attempting to connect to new WiFi network <<<<<<");
	Serial.print("Connecting...");
	while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
		web_server.send(200, "text/plain", "");
		Serial.print(".");
		delay(100);
	}
	Serial.println("");

	if (WiFi.status() != WL_CONNECTED) {
		Serial.print(">>>>>> Failed to connect <<<<<<");
		Access_Point_SetUp();

	}
	else {
		Serial.println(">>>>>> Successfully connected to new network <<<<<<");
		Serial.print(">>>>>> Check if im on the network! My IP adress is: ");
		Serial.print(WiFi.localIP());
		Serial.println(" <<<<<<");
		Serial.print(">>>>>> Network name: ");
		Serial.print(WIFI_NETWORK);
		Serial.println(" <<<<<<");
		web_server.send(200, "text/plain", "Connected!");
		delay(100);
	}
}
void handle_OnConnect() {
	LED1status = LOW;
	LED2status = LOW;
	Serial.println(">>>>>> GPIO4 Status: OFF | GPIO5 Status: OFF <<<<<<");
	web_server.send(200, "text/html", SendHTML_main(LED1status, LED2status));
}

void handle_led1on() {
	LED1status = HIGH;
	Serial.println("GPIO4 Status: ON");
	web_server.send(200, "text/html", SendHTML_main(true, LED2status));
}

void handle_led1off() {
	LED1status = LOW;
	Serial.println("GPIO4 Status: OFF");
	web_server.send(200, "text/html", SendHTML_main(false, LED2status));
}

void handle_led2on() {
	LED2status = HIGH;
	Serial.println("GPIO5 Status: ON");
	web_server.send(200, "text/html", SendHTML_main(LED1status, true));
}

void handle_led2off() {
	LED2status = LOW;
	Serial.println("GPIO5 Status: OFF");
	web_server.send(200, "text/html", SendHTML_main(LED1status, false));
}

void handle_joinNetwork() {
	web_server.send(200, "text/html", SendHTML_login());
}

void handle_joinedNetwork() {
	//Retrieve the netowrk name and password in the URL's arguments (?Arg1=value&Arg2=value&...)
	// -- NOTE: for whatever reason you have to store the argument in a string then convert you can't just convert i.e. WIFI = server.arg(0).c_str() doesn't work
	String net, pass;
	net = web_server.arg(0);
	pass = web_server.arg(1);
	WIFI_NETWORK = net.c_str();
	WIFI_PASSWORD = pass.c_str();


	Serial.print("WIFI NETWORK: ");
	Serial.println(WIFI_NETWORK);
	Serial.print("WIFI PASSWORD: ");
	Serial.println(WIFI_PASSWORD);
	web_server.send(200, "text/plain", "");
	Serial.print("ATTEMPTING TO CONNECT TO ANOTHER NETWORK");
	connect_to_WiFi();

}

void handle_NotFound() {
	//Tries to join a page which doesn't exist
	web_server.send(404, "text/plain", "Not found");
}


void Access_Point_SetUp() {
	//-- TODO add personalised set up
	const char* ssid = "Module Check";  // Enter SSID here
	const char* password = "123";  //Enter Password here

	IPAddress local_ip(192, 168, 1, 1);
	IPAddress gateway(192, 168, 1, 1);
	IPAddress subnet(255, 255, 255, 0);

	pinMode(LED1pin, OUTPUT);
	pinMode(LED2pin, OUTPUT);
	WiFi.softAP(ssid, password);
	WiFi.softAPConfig(local_ip, gateway, subnet);
	delay(100);
	Nexus_IP_Address = local_ip;

	web_server.on("/", handle_OnConnect);
	web_server.on("/led1on", handle_led1on);
	web_server.on("/led1off", handle_led1off);
	web_server.on("/led2on", handle_led2on);
	web_server.on("/led2off", handle_led2off);
	web_server.on("/Join", handle_joinNetwork);
	web_server.on("/Joined", handle_joinedNetwork);

	web_server.onNotFound(handle_NotFound);
	web_server.begin();

	other_networks = get_WiFi_list().c_str();

	Serial.println(">>>>>> HTTP server started <<<<<<");
	Serial.print(">>>>>> PLEASE CONNECT TO IP ADDRESS  ");
	Serial.print(Nexus_IP_Address);
	Serial.println(" ON YOUR WEB BROWSER <<<<<<");
	delay(2000);

	while (true) {
		web_server.handleClient();
		if (LED1status)
		{
			digitalWrite(LED1pin, HIGH);
		}
		else
		{
			digitalWrite(LED1pin, LOW);
		}

		if (LED2status)
		{
			digitalWrite(LED2pin, HIGH);
		}
		else
		{
			digitalWrite(LED2pin, LOW);
		}
	}

}


void setup() {
	Serial.begin(115200);
	Access_Point_SetUp();
}

// the loop function runs over and over again until power down or reset
void loop() {

}
