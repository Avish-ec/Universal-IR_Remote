// Including libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include<PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// Defining IR transmitter at pin D5
const uint16_t kIrLed = D5;
IRsend irsend(kIrLed); 

//SSID and Password of your WiFi router
const char* ssid = "realme C11";
const char* password = "3db72ef563eb";

const char* ssid_user = "";
const char* password_user = "";

// Set broker for MQTT client
const char* mqtt_server = "broker.hivemq.com";

unsigned long millisNow = 0; //for delay purposes
unsigned int sendDelay = 2000; //delay before sending sensor info via MQTT

// Set MQTT client
WiFiClient espClient;
PubSubClient client(espClient);
 

ESP8266WebServer server(80); //Server on port 80

// Declare variables
int Tnetwork=0,i=0,total=0;
String s="", ss= "",pp="",ssid_names[20];


///////////////// HTML page////////////////////////////
const char index_html[] = R"rawliteral(
<!DOCTYPE HTML><html>
    <head>
    <title>ESP Input Form</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.3.0/font/bootstrap-icons.css" />
    <link rel="stylesheet" href="css/style.css" />
    <link href="https://fonts.googleapis.com/css?family=Open+Sans" rel="stylesheet">
    <style>
        body{
            background-image: url('https://cdn.wallpapersafari.com/14/49/XEQYeN.jpg');
            background-repeat: no-repeat;
            background-position: fixed;
            background-size: cover ;
            font-family: "Open Sans";
            width : 100%;

            height : auto;
            font-size: 1.2vw;

        }
        .canvas{
            width : 35%;
            background-color: rgba(129, 127, 96, 0.5);
            padding : 2.1961932650073206vw;
            backdrop-filter: blur(1px);
            transform: translateX(10%);
        }
        input,select{
            width: 90%;
            border-top: none;
            border-right: none;
            border-left: none;
            border-bottom: 0.21961932650073207vw solid rgb(255, 255, 255);
            outline: none;
            background: none;
            height: 2.9282576866764276vw;
            color: white;
            font-family: "Open Sans";
            font-size: 1.2vw;

        }
        button{
            border: none;
            background-color: white;
        }
        option{
            color: rgb(255, 255, 255);
            font-family: "Open Sans";
            background-color: grey;
        }
        label{
            color : white;
display: block;
padding-top:  0.7320644216691069vw;

        }
        @media only screen and (min-width: 600px) {
  body {  
     background-image: url("https://browsecat.net/sites/default/files/desktop-pattern-geometric-wallpapers-104752-824285-4301571.png");
            background-repeat: no-repeat;
            background-position: fixed;
            background-size: cover ;
            font-family: "Open Sans";
            width : 100%;
            height : auto;
  }
}
        </style>
    

        </head>
        <body>
            <div class="canvas">
                <form action="/get"> 
                    <label>Select SSID</label>   
                <select name="names">  
                )rawliteral";
  const char down_html[] = R"rawliteral(</select>
                   <label>Password:</label> <input type="password" name="psw" id="typepass">
                   <i class="bi bi-eye-slash" id="togglePassword" style="margin-left: -2.1961932650073206vw; cursor: pointer;"></i>
                    
                   <br><br>
                   <input type="submit" value="Submit" style="border: none; background-color: white; color: black;
                   font-family: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif; font-weight: bold ; font-size: 1.171303074670571vw;"></P>
                 </form>
                    </div>
                    <script>
                        // Change the type of input to password or text
                        const togglePassword = document.querySelector("#togglePassword");
        const password = document.querySelector("#typepass");

        togglePassword.addEventListener("click", function () {
            // toggle the type attribute
            const type = password.getAttribute("type") === "password" ? "text" : "password";
            password.setAttribute("type", type);
            
            // toggle the icon
            this.classList.toggle("bi-eye");
        });
                    </script>

        </body>
    </html>)rawliteral";
//================================================================================================================


//==============================================================
//                          SUBSCRIBE
//==============================================================
// Function which subscribe to a topic
void subscriber(){
  client.subscribe("MYdata");
//  client.subscribe("topic2");
//  client.subscribe("topic3");
}


//==============================================================
//                          CALLBACK
//==============================================================
// This function is called when there is an incoming message in topic 
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("\n              Message from callback Function : Data arrived in topic [");
  Serial.print(topic);
  Serial.print("] : ");
  // A loop to retrive each character in message
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    // Send retrived data through Ir Tx
    irsend.sendNEC((char)message[i]);
  }
  Serial.println();
}


//==============================================================
//                          RECONNECT
//==============================================================
// This function connects MQTT client to broker
void reconnect() {
  // Loop until we're reconnected
  int counter = 0;
  while (!client.connected() && (WiFi.status() == WL_CONNECTED)) {
    if (counter==5){
      ESP.restart();
    }
    counter+=1;
    Serial.print("\n              Message from reconnect function : Attempting MQTT connection...");
    // Attempt to connect
    
    // Attempt to connect to MQTT broker
    if (client.connect("0867a4a6-7461-4afd-800c-f794298acc69","Avish" , "123456")) {
      Serial.println("connected");
      // Calling subscriber function after connection
      subscriber(); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
}



//==============================================================
//                         SETUP_WIFI
//==============================================================
void setup_wifi(){
  // Declare state for selection of SSID and password
  boolean state = true;
  String temp_ssid, temp_password ;

  // Set station mode
  WiFi.mode(WIFI_STA);
  Serial.println("             Message from setup_wifi function : WiFi connecting");

 again: int counttime = 0;

 // Assign SSID and password according to state
 if (state){
     temp_ssid = ssid;
     temp_password = password;

  }
  else{
     temp_ssid = ss;
     temp_password = pp;
  }
  
  WiFi.begin(temp_ssid, temp_password);     //Connect to your WiFi router
  Serial.println("");
  Serial.print("             ");

  // Wait for connection
  while ((WiFi.status() != WL_CONNECTED) && (counttime <= 15)) {
    delay(500);
    Serial.print(".");
    counttime += 1;
  }
  // Change state regardless of connection
  state = false;

  // Check for connection
  if (WiFi.status() != WL_CONNECTED){
    // If not connected go to again to re-execute 
    goto again;
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("              Connected to ");
  Serial.println(temp_ssid);
  Serial.print("              IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

}


//==============================================================
//                          HANDLEARG
//==============================================================
// This function is called to retrieve data 
void handle_arg(){
  Serial.println("\n              Request sent for agument query through handle_arg function ");

  // Searches argument in server
  ss = server.arg("names");
  pp = server.arg("psw");
  Serial.print("\n              Got ssid and password : ");
  Serial.print(ss);
  Serial.print(" & ");
  Serial.print(pp);

  // Send response to client after retrieval
  server.send(200, "text/html", "Your response has been recorded <a href='/\'>Go to homepage</a>");
}



//==============================================================
//                          HANDLEROOT
//==============================================================
// This function is called after an incoming client request
void handleRoot(){
  Serial.print("\n              Message from handleroot function : [");

  // A for loop to print nearby network to serial monitor
 for(int r=0;r<Tnetwork;r++){
    Serial.print(ssid_names[r]);
    Serial.print(", ");
  }
  Serial.print(" ]");
  server.send(200, "text/html",s );

  // A for loop to clear stored strings
  for (int c=0;c<Tnetwork;c++){
    ssid_names[c]="";
  }

}


//==============================================================
//                          SETUP
//==============================================================
void setup(void){

  // Initiate serial monitor at 9600 baud rate
  Serial.begin(9600);

  // Call setup WiFi function
  setup_wifi();
  server.on("/", handleRoot);      //Which routine to handle at root location

  server.begin();                  //Start server
  Serial.println("              Message from setup function : HTTP server started");
  server.on("/get",handle_arg);

  // Set MQTT server
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  irsend.begin();
  
  }


//===============================================================
//                            LOOP
//===============================================================

void loop(void){
  // Scan available networks and store it in a string array
    Tnetwork = WiFi.scanNetworks();
    for (int i = 0; i < Tnetwork; ++i){
        ssid_names[i] = WiFi.SSID(i);
    }

    // Append upper body of HTML in S string
    s = index_html;

    // A loop for HTML Option tag ( To enter values in <option> tag ) which appends in string s
    for (int op=0;op<Tnetwork;op++){
        s += "<option value='";
        s += ssid_names[op];
        s += "'>";
        s += ssid_names[op];
        s += "</option>";
    }

    // Append lower body of HTML in S string
    s += down_html;
    server.handleClient();          //Handle client requests

    // Clear string after client request
    s = "";

    // Check for WiFi connection
    if (WiFi.status() != WL_CONNECTED){

      // If not connected call setup function
        setup_wifi();
    }

    // Check for broker connection
    if (!client.connected()) {

      // If not connected call reconnect function
        reconnect();
    }

    // Start thread
    client.loop();
}
  

