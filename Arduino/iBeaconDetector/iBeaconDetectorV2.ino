#include <SoftwareSerial.h>
#include <Bounce2.h>

#define LED_R_PIN 4
#define LED_G_PIN 5
#define LED_Y_PIN 6
#define SWITCH_PIN 7

// SoftwareSerial(rxPin, txPin, inverse_logic)
// RX Pin 10 for Receive,TX Pin 9 for Send
SoftwareSerial hm_10(10, 9);
// 暫存變數
char chr;
static int i;
// 解決硬體開關彈跳
Bounce bouncer = Bounce();
boolean status;

// 根據傳入參數亮燈
// "01"亮紅燈  "02" 亮綠燈  "03" 亮黃燈  "00"不亮燈
void ligth_up_led(String str){
  // 檢查傳入參數是不是為2個字元，否則將參數設為"00"
  if(!str.length() == 2){
    str = "00";
  }

  if(str == "01"){
    digitalWrite(LED_R_PIN, HIGH);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_Y_PIN, LOW);
  }else if(str == "02"){
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, HIGH);
    digitalWrite(LED_Y_PIN, LOW);
  }else if(str == "03"){
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_Y_PIN, HIGH);
  }else{
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_Y_PIN, LOW);
  }
}

void setup(){
  // pin設定
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_Y_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT);
  // Bounce設定
  bouncer.attach(SWITCH_PIN);
  bouncer.interval(5);
  status = false;
  // baud率設定
  Serial.begin(115200);
  hm_10.begin(115200);

  Serial.println("HM-10 is begin listening"); 
}

void loop(){
  String response = String("");
  String uuid = String("");
  String major = String("");
  String minor = String("");
  // 在response中找尋的iBeacon關鍵字串
  String searchCondi = "OK+DISC:4C000215";
  
  // 按鈕開關改變 status
  if(bouncer.update() && bouncer.read() == HIGH){
    status = !status;
  }

  // Arduino 如果收到 Serial 來的命令，則傳給 HM-10
  // 用於下 AT Command 指令
  while(Serial.available() > 0){
    chr = Serial.read();
    hm_10.print(chr);
  }
  
  // Arduino 如果收到 HM-10 來的回應
  // 則將回應整理為 response 字串
  // 於後面判斷
  while(hm_10.available()){
    chr = hm_10.read();
    response += chr;
  }

  // 如果 response 有內容
  if(response != ""){

    // 處理 response 字串，確認回傳字串是否為目標字串
    if(response.substring(0,15) == searchCondi){

      // 取得response字串內uuid,major,minor
      uuid = response.substring(16,49);
      major = response.substring(50,53);
      minor = response.substring(54,57);

      // 根據 status 判斷要顯示哪組紅綠燈
      if(status){
        ligth_up_led(minor.substring(0,1));
      }else{
        ligth_up_led(minor.substring(2,3));
      }
      
      // 清除字串
      uuid = "";
      major = "";
      minor = "";
    }

    response = "";
  }

  // 查詢附近iBeacon
  if(!hm_10.available()){
    i++;
    // 硬體會跟不上程式，所以加上這行
    delay(1); 
    // 1*1000 = 1，每1秒查詢一次
    if(i%1000==0){
      hm_10.print("AT+DISI?");
      hm_10.flush();
      i=0;
    }
  }
}