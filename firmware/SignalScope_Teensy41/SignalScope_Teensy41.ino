/*
 SignalScope Teensy 4.1 bridge firmware v0.1
 Tools > USB Type: Serial. Select Teensy 4.1 in Teensyduino.
 USB Serial is the browser link. Serial1 RX pin 0 receives target UART.
 GPIO capture uses pins 2, 3, 4, 5. Host commands are newline-delimited.
*/
#include <Arduino.h>
static const uint8_t GPIO_PINS[] = {2, 3, 4, 5};
static const uint8_t GPIO_N = 4;
static const uint16_t EDGE_N = 64;
struct Edge { uint8_t pin; uint8_t level; uint32_t us; };
volatile Edge edgeQ[EDGE_N];
volatile uint8_t edgeHead = 0, edgeTail = 0;
volatile uint32_t edgeDrops = 0;
char commandLine[160];
uint8_t commandLen = 0;
uint32_t uartBaud = 115200;
bool gpioMode = false;

void edgePush(uint8_t pin) {
  uint8_t next = (edgeHead + 1) % EDGE_N;
  if (next == edgeTail) { edgeDrops++; return; }
  edgeQ[edgeHead].pin = pin; edgeQ[edgeHead].level = digitalReadFast(pin); edgeQ[edgeHead].us = micros();
  edgeHead = next;
}
void isr2(){ edgePush(2); } void isr3(){ edgePush(3); }
void isr4(){ edgePush(4); } void isr5(){ edgePush(5); }

void statusMsg(bool ok, const char *message) {
  Serial.print("{\"type\":\"status\",\"ok\":"); Serial.print(ok ? "true" : "false");
  Serial.print(",\"message\":\""); Serial.print(message); Serial.println("\"}");
}
void hello() {
  Serial.print("{\"type\":\"hello\",\"firmware\":\"0.1\",\"board\":\"Teensy 4.1\",");
  Serial.println("\"capabilities\":[\"UART_RX\",\"UART_TX\",\"GPIO_EDGE\",\"GPIO_CAPTURE\"]}");
}
void emitUart(uint8_t b) {
  Serial.print("{\"type\":\"frame\",\"bus\":\"UART\",\"direction\":\"RX\",\"time_us\":");
  Serial.print(micros()); Serial.print(",\"hex\":\"");
  if (b < 16) Serial.print('0'); Serial.print(b, HEX);
  Serial.print("\",\"text\":\"");
  if (b >= 32 && b <= 126 && b != '"' && b != '\\') Serial.write(b);
  else if (b == '"' || b == '\\') { Serial.write('\\'); Serial.write(b); }
  else Serial.print('.');
  Serial.print("\",\"wave\":\"0");
  for (uint8_t i=0;i<8;i++) Serial.print((b >> i) & 1);
  Serial.println("1\"}");
}
void emitEdges() {
  while (edgeTail != edgeHead) {
    Edge e;
    noInterrupts(); e.pin=edgeQ[edgeTail].pin; e.level=edgeQ[edgeTail].level; e.us=edgeQ[edgeTail].us; edgeTail=(edgeTail+1)%EDGE_N; interrupts();
    Serial.print("{\"type\":\"edge\",\"pin\":"); Serial.print(e.pin);
    Serial.print(",\"level\":"); Serial.print(e.level ? "1" : "0");
    Serial.print(",\"time_us\":"); Serial.print(e.us); Serial.println("}");
  }
}
void configureGpio() {
  gpioMode=true;
  pinMode(2,INPUT); pinMode(3,INPUT); pinMode(4,INPUT); pinMode(5,INPUT);
  attachInterrupt(digitalPinToInterrupt(2),isr2,CHANGE);
  attachInterrupt(digitalPinToInterrupt(3),isr3,CHANGE);
  attachInterrupt(digitalPinToInterrupt(4),isr4,CHANGE);
  attachInterrupt(digitalPinToInterrupt(5),isr5,CHANGE);
}
void captureGpio(uint32_t intervalUs, uint16_t count) {
  if (count<8) count=8; if (count>1024) count=1024;
  if (intervalUs<20) intervalUs=20; if (intervalUs>100000) intervalUs=100000;
  uint8_t samples[1024];
  for (uint16_t i=0;i<count;i++) {
    uint8_t v=0;
    for(uint8_t c=0;c<GPIO_N;c++) if(digitalReadFast(GPIO_PINS[c])) v|=(1<<c);
    samples[i]=v; delayMicroseconds(intervalUs);
  }
  Serial.print("{\"type\":\"waveform\",\"bus\":\"GPIO\",\"interval_us\":");
  Serial.print(intervalUs); Serial.print(",\"pins\":[2,3,4,5],\"samples\":[");
  for(uint16_t i=0;i<count;i++){ if(i)Serial.print(','); Serial.print(samples[i]); }
  Serial.println("]}");
}
void executeCommand(char *line) {
  if (!strcmp(line,"HELLO")) { hello(); return; }
  if (!strcmp(line,"MODE GPIO")) { configureGpio(); statusMsg(true,"GPIO edge mode active on pins 2, 3, 4, 5"); return; }
  if (!strncmp(line,"MODE UART ",10)) {
    uint32_t b=atoi(line+10); if(b<300 || b>3000000){statusMsg(false,"Invalid UART baud rate");return;}
    gpioMode=false; for(uint8_t i=0;i<GPIO_N;i++)detachInterrupt(digitalPinToInterrupt(GPIO_PINS[i]));
    uartBaud=b; Serial1.begin(uartBaud); statusMsg(true,"UART RX active on pin 0"); return;
  }
  if (!strncmp(line,"CAPTURE ",8)) {
    unsigned long us=0; unsigned int n=512; sscanf(line+8,"%lu %u",&us,&n); captureGpio((uint32_t)us,(uint16_t)n); return;
  }
  if (!strncmp(line,"TXHEX ",6)) {
    char *p=line+6; while(*p){while(*p==' '||*p==',')p++; if(!*p)break; char *end; long v=strtol(p,&end,16); if(end==p)break; Serial1.write((uint8_t)v); p=end;} return;
  }
  if (!strncmp(line,"TX ",3)) { Serial1.write((const uint8_t *)(line+3),strlen(line+3)); Serial1.write('\r'); Serial1.write('\n'); return; }
  statusMsg(false,"Unknown command");
}
void setup() { Serial.begin(115200); Serial1.begin(uartBaud); }
void loop() {
  while(Serial.available()) {
    char c=Serial.read();
    if(c=='\r')continue;
    if(c=='\n'){commandLine[commandLen]=0;executeCommand(commandLine);commandLen=0;}
    else if(commandLen<sizeof(commandLine)-1)commandLine[commandLen++]=c;
    else commandLen=0;
  }
  if(!gpioMode) while(Serial1.available()) emitUart((uint8_t)Serial1.read());
  else emitEdges();
}
