#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

/* optional set host name */
//#define HOST_NAME "arduino.amapz.com"

EthernetClient webClient;
EthernetServer server(80);
String strResponse = "";

int NO = 0;
int NC = 1;

/* FIXME loop array twice too big */
int numberOfRelays = 2;
int relays[] = {8, 9};
int states[]= {-1, -1};

void setup() {
  Serial.begin(115200);
Serial.println("Proto3 - Jean-Philippe@Ruijs.fr");
  testRelays();

  startServer();
}

void startServer(){
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x2D};

  if (Ethernet.begin(mac) == 0) {
    Serial.print("Static IP: ");
    IPAddress ip(192,168,0,203);
    IPAddress gateway(192, 168, 0, 1);
    IPAddress subnet(255, 255, 255, 0);
    Ethernet.begin(mac, ip, gateway, subnet);
  }else{
    Serial.print("DHCP IP: ");
  }
  Serial.println(Ethernet.localIP());
  server.begin();
}

void loop(){
  //loopWebClient();
  loopWebServer();
}

void loopWebServer() {
  EthernetClient serverClient = server.available();
  if (serverClient) {
    Serial.println("new serverClient");

    String requestStr = "";
    boolean gotGet = false;
    boolean currentLineIsBlank = true;

    while (serverClient.connected()) {
      if (serverClient.available()) {
        
        char c = serverClient.read();

        /* an http request ends with a blank line */
        if (c == '\n' && currentLineIsBlank) {
          printRequest(requestStr);
          break;
        }

        /* you're starting a new line */
        if (c == '\n') {
          currentLineIsBlank = true;
          requestStr.concat(c); //preserve new line
          /* got a GET request */
          if(!gotGet && requestStr.indexOf("GET /") > -1){
            gotGet = true;
            
            if( isRelayRoute(requestStr) ){
              restRoute(requestStr);
              response(serverClient, "application/json");
            }
          }
        }
         /* you've gotten a character on the current line */
        else if (c != '\r') {
          currentLineIsBlank = false;
          requestStr.concat(c);
        }
      }
    }
  }
  delay(10);
  serverClient.stop();
}


void restRoute(String strGet){
  //Serial.print("restRoute()::");
  int SLASH_RELAY_CHILD = 10;
  strResponse.concat("{\n");
    //route relay
    for(int i=0; i < numberOfRelays; i++){
      if( isRelayID(strGet,i, SLASH_RELAY_CHILD) ){
        if( isChild(strGet,"/state", SLASH_RELAY_CHILD) )  {
          strResponse.concat(getHeader("state"));
        }
        if( isChild(strGet,"/toggl", SLASH_RELAY_CHILD) )  {
          toggle(i);
          //strResponse.concat(getHeader("toggle"));
        }
        if( isChild(strGet,"/low", SLASH_RELAY_CHILD) )     {
          goLow(i);
          //strResponse.concat(getHeader("low"));
        }
        if( isChild(strGet,"/high", SLASH_RELAY_CHILD) )    {
          getHigh(i);
          strResponse.concat(getHeader("high"));
        }
        strResponse.concat(getRelayState(i));
        strResponse.concat("\n");
      }
    }
    //route info
    if( strGet.indexOf("/info") == SLASH_RELAY_CHILD ){
      strResponse.concat(getHeader("info"));
      String strStates = "";
      for(int i=0; i < numberOfRelays; i++){
        strStates.concat(getRelayState(i));
        if(i<(numberOfRelays-1)) strStates.concat(",");
        strStates.concat("\n");
      }
      strResponse.concat("\"");
      strResponse.concat("relays");
      strResponse.concat("\":\n{");
      strResponse.concat(strStates);
      strResponse.concat("}");
    }
    
  strResponse.concat("\n}");
}

void testRelays(){
  Serial.print(sizeof(relays));
  Serial.print(", hardcoded: ");
  Serial.println(numberOfRelays);

  for(int i=0; i < numberOfRelays; i++){
    pinMode(relays[i], OUTPUT);
    goLow(i);
    delay(250);
    getHigh(i);
    delay(250);
  }
}

void toggle(int i){
  if(states[i] == HIGH){ goLow(i); }
  else{
    if(states[i] == LOW){ getHigh(i); }
  }
}

void getHigh(int i){
  digitalWrite(relays[i], HIGH);
  states[i]=HIGH;
  delay(100);
}

void goLow(int i){
  digitalWrite(relays[i], LOW);
  states[i]=LOW;
  delay(100);
}

String getHeader(String action){
  String h = "\"header\": {";
  h.concat(getNV("action", action));
  h.concat(", ");
  h.concat(getNV("server_ip", getStringIP(Ethernet.localIP())));
  h.concat("}\n, ");
  return h;
}

String getPinState(int i, String task){
  String s = "";
//  s.concat(getNV(task+"_id", String(i)));
  s.concat("\"");
    s.concat(task+String(i));
  s.concat("\"");
  
  s.concat(": {");
  s.concat(getNV("pin_id", String(relays[i])));

  s.concat(", ");
  s.concat(getNV("state", String(states[i])));
  
  int state = states[i];
  s.concat(", ");
  if(state == 0){
    s.concat(getNV("is", "low"));
  }else{
    s.concat(getNV("is", "high"));
  }
  
  s.concat(" }");
  return s;
}

String getRelayState(int i){
  return getPinState(i,"relay"); 
}

String getRouteRelay(int i){
  String s = "/";
  s.concat(String(i));
  s.concat("/");
  return s;
}

String getNV(String n, String v ){
  String r = "\"";
  r.concat(n);
  r.concat("\": \"");
  r.concat(v);
  r.concat("\"");
  return r;
}

String getStringIP(IPAddress address){
  String strIP = String(address[0]);
  strIP.concat(".");
  strIP.concat(String(address[1]));
  strIP.concat(".");
  strIP.concat(String(address[2]));
  strIP.concat(".");
  strIP.concat(String(address[3]));
  return strIP;
}

bool isRelayRoute(String strGet){
  return isRouteRoute(strGet, "/relay");
}

bool isRouteRoute(String stack, String needle){
  int SLASH_RELAY = 4;
  return stack.indexOf(needle) == SLASH_RELAY;
}

bool isRelayID(String strGet, int i, int idx){
  return strGet.indexOf(getRouteRelay(i)) == idx;
}

bool isChild(String strGet, String childRoute, int idx){
  return strGet.indexOf(childRoute) > idx;
}

void printRequest(String requestStr){
  Serial.println("\n__REQ start___");
  Serial.println(requestStr);
  Serial.println("__REQ end___\n");
}
/* standard http 1.1 200 response*/
void response(EthernetClient serverClient, String contentType){
  Serial.println("__RESP start___");
  serverClient.println("HTTP/1.1 200 OK");
  serverClient.println("Access-Control-Allow-Origin: *");
  serverClient.print("Content-Type: ");
  serverClient.println(contentType);
  serverClient.println("Connection: close");
  serverClient.println();
  serverClient.println(strResponse);
  Serial.println(strResponse);
  Serial.println("__RESP end___");
  strResponse = "";
}

