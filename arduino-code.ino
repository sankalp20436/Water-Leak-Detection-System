#include <SoftwareSerial.h>

#include <stdlib.h>
#define DEBUG true
#define RX 10
#define TX 11

String HOST = "api.thingspeak.com";
String PORT = "80";
String AP = "JioFi2_196C3E";
String PASS = "yhu5epzpac";


String API = "5TPNJ28RVLBUPIAM";
String field = "";

int countTrueCommand;
int countTimeCommand;
boolean found = false;
int sensorInterrupt = 0;  // interrupt 0
int sensorPin       = 2; //Digital Pin 2
int solenoidValve = 5; // Digital pin 5
unsigned int SetPoint = 400; //400 milileter
 
//The hall-effect flow sensor outputs pulses per second per litre/minute of flow.//
float calibrationFactor = 4.2; //You can change according to your datasheet
 
volatile byte pulseCount =0;  
 
int flowRate = 0;
int flowLitres =0;
int totalLitres = 0;
 
unsigned long oldTime = 0;
 


SoftwareSerial esp8266(RX,TX);
void setup() {

Serial.begin(9600);
esp8266.begin(115200);
pinMode(solenoidValve , OUTPUT);
digitalWrite(solenoidValve, HIGH);
pinMode(sensorPin, INPUT);
digitalWrite(sensorPin, HIGH);
attachInterrupt(sensorInterrupt, pulseCounter, FALLING);




sendCommand("AT",5,"OK");
/*sendCommand("ATE0",5,"OK");*/
sendCommand("AT+CWMODE=1",5,"OK");
sendCommand("AT+CWQAP",1000,"OK");
 Serial.println("Connecting Wifi....");

sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",15,"OK");

countTrueCommand = 0;




 


}
void loop() {
  
/*String getData = "GET /update?api_key="+ API +"&"+ field 1 +"="+String(flowLitres)+"&"+ field 2 +"="+String(totalLitres);*/
 String getData = "GET /update?api_key="+ API +"&field1="+String(flowLitres)+"&field3="+String(totalLitres);
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);delay(1500);countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");

if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    
    detachInterrupt(sensorInterrupt);
 
    
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
 
    
    oldTime = millis();
 
    
    flowLitres = (flowRate / 60) * 1000;
 
    
    totalLitres += flowLitres;
 
    unsigned int frac;
 
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(flowLitres );
    Serial.print("L/MIN");
    Serial.print("\t");           
 
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalLitres);
    Serial.println("L"); 
    Serial.print("\t");     
        
    if (totalLitres > 40)
    {
      SetSolinoidValve();
    }
    
// Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
 
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
delay(10000);


}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
 
void SetSolinoidValve()
{
  digitalWrite(solenoidValve, LOW);
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OK");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
