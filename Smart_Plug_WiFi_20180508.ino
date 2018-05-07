#include <ESP8266WiFi.h>
#include <Thread.h>
#include <ThreadController.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
ThreadController controll = ThreadController();
Thread myThread = Thread();
Thread myThread2 = Thread();

const char* ssid = "Your SSID of AP";
const char* password = "Your Password of AP";
int relayPin = D5;
int ledPin = 2;
int pin = D1;
int current;
double kilos = 0;
int peakPower = 0;

WiFiServer server(80);

void setup() 
{
 Serial.begin(115200);
 delay(10);
 pinMode(ledPin, OUTPUT);
 digitalWrite(ledPin, HIGH);
 pinMode(pin, OUTPUT);
 pinMode(relayPin, OUTPUT);
 digitalWrite(relayPin, LOW);

 // Set WiFi to station mode and disconnect from an AP if it was previously connected
 WiFi.mode(WIFI_STA);
 WiFi.disconnect();
 delay(100);
 Serial.println("Setup done");

 // Connect to WiFi network
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);
 WiFi.begin(ssid, password);
   
 while (WiFi.status() != WL_CONNECTED) 
 {
   delay(500);
   Serial.print(".");
 }
   
 Serial.println("");
 Serial.println("WiFi connected");
 
 // Start the server
 server.begin();
 Serial.println("Server started");
   
 // Print the IP address
 Serial.print("Use this URL to connect: ");
 Serial.print("http://");
 Serial.print(WiFi.localIP());
 Serial.println("/");

 lcd.begin(16,2);
 lcd.init();
 lcd.setCursor(0, 0);
 lcd.print("Smart Plug");
 lcd.setCursor(0, 1);
 lcd.print(WiFi.localIP());
 lcd.backlight();
 delay(5000);  
 lcd.setCursor(0, 0);
 lcd.print("*** Waiting! ***"); 

 myThread.onRun(webServer);
 myThread.setInterval(1500);

 myThread2.onRun(powerCheck);
 myThread2.setInterval(1000);

 controll.add(&myThread);
 controll.add(&myThread2);
}

void loop() 
{
  controll.run();
}

void webServer()
{
   
   // Check if a client has connected
   WiFiClient client = server.available();
   if (!client) 
   {
     return;
   }
   
   // Wait until the client sends some data
//  Serial.println("new client");

  
   while(!client.available())
   {
     delay(1);
   }
   
   // Read the first line of the request
   String request = client.readStringUntil('\r');
   Serial.println(request);
   
   // favicon.ico request이면 건너 뛰자.
  if( request.substring(5,16) != "favicon.ico") 
   {
      client.flush();
   
      // Match the request
      if (request.indexOf("/PLUG=ON") != -1)  
         {
              digitalWrite(ledPin, LOW);
              digitalWrite(relayPin, HIGH);
              lcd.setCursor(0, 0);
              lcd.print("Status = ON     ");
         }
   
      if (request.indexOf("/PLUG=OFF") != -1)  
         {
              digitalWrite(ledPin, HIGH);
              digitalWrite(relayPin, LOW);
              lcd.setCursor(0, 0);
              lcd.print("Status = OFF    ");
         } 
      
             // Return the response
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println(""); //  do not forget this one
             client.println("<!DOCTYPE HTML>");
             client.println("<html>");
             client.println("<head>");
          
             //배경 색 문자 색 사이즈 HTML CSS 설정   
             client.println("<style>");
             client.println("body {");
             client.println("background-color: white;");
             client.println("}");
             client.println("p {");
             client.println("color:green;");
             client.println("font-size:300%;");
             client.println("}");
             
             //버튼 HTML CSS 설정   
             client.println(".button {");
             client.println("background-color: yellow;");
             client.println("border: none;");
             client.println("color: black;");
             client.println("padding: 20px 32px;");
             client.println("text-align: center;");
             client.println("text-decoration: none;");
             client.println("display: inline-block;");
             client.println("font-size: 20px;");
             client.println("margin: 14px 20px;");
             client.println("cursor: pointer;");
             client.println("}");
              
             client.println("</style>");
             client.println("</head>");
             client.println("<body>");
             client.println("<p>");
             client.print("Smart Plug");
             client.println("<br>");
             
             client.println("<style>");
             client.println("p {");
             client.println("color:blue;");
             client.println("font-size:300%;");
             client.println("}");
             client.println("</style>");          
             client.print("Made by Jason");              
             client.println("<br>");
             client.println("<a href=\"/PLUG=ON\"\" class='button'>Plug on </button></a>");//Built-in LED on
             client.println("<a href=\"/PLUG=OFF\"\" class='button'>Plug Off </button></a>");//Built-in LED OFF
             client.println("</p>");
             client.println("</body>");
             client.println("</html>");
             delay(1);
           
             Serial.println("Client disonnected");
             Serial.println("");
    }         // if문 괄호 닫기
}

void powerCheck()
{
  int current = 0;
  int maxCurrent = 0;
  int minCurrent = 1023;
  
        for (int i=0 ; i<=2048 ; i++)  //Monitors and logs the current input for 200 cycles to determine max and min current
        {
          current = analogRead(A0);    //Reads current input and records maximum and minimum current
          if(current >= maxCurrent)
             maxCurrent = current;
          else if(current <= minCurrent)
            minCurrent = current;
            delayMicroseconds(10);  
        }
  
//      Serial.println(maxCurrent);  
        if (maxCurrent <= 431)
        {
          maxCurrent = 431;
        }

        double RMSCurrent = ((maxCurrent - 431)*0.707*6.6/220/6);    
        
        int RMSPower = 220*RMSCurrent;    //Calculates RMS Power Assuming Voltage 220VAC, change to 110VAC accordingly
        if (RMSPower > peakPower)
        {
          peakPower = RMSPower;
        }
        kilos = kilos + (RMSPower * (2.05/60/60/1000));    //Calculate kilowatt
        
       lcd.setCursor(0,1);
       lcd.print("                ");       
       
       lcd.setCursor(0,1);
       lcd.print(kilos);
       lcd.print("kWh");       

       lcd.setCursor(11,1);
       lcd.print(RMSPower);
       lcd.print("W");
}










