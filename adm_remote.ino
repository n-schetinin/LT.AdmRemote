#include <U8glib.h>

#include <IRremote.h>
#include <IRremoteInt.h>                                                     

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);  

const int btnB = 5; //Back button
const int btnM = 6; //Menu button
const int btnN = 7; //Next button

//Current and previous buttons states
int curBtnB = 0;
int curBtnM = 0;
int curBtnN = 0;
int prBtnB = 0;
int prBtnM = 0;
int prBtnN = 0;

int mode; //working mode (0 - menu, 1 - displaying shot)

IRrecv irrecv(2); //Pin 2 - IR recieve
IRsend irsend; //Always pin 3
decode_results results; 

//Display strings
String str1=""; 
String str2="";
String str3="";

int curMenu; //Current menu index

const int arrCnt=13;

long commands[arrCnt] ={
  0x8302E8,  
  0x8300E8,  
  0xA900E8,
  0xA901E8,
  0xA902E8,
  0xA903E8,
  0x83F9E8,
  0x83F8E8,
  0x830DE8, //?? full health
  0x830FE8, //?? full ammo
  0x8A01E8,
  0x8320E8,
  0x8322E8


};

String names[arrCnt]={
  "New game",
  "Admin kill",
  "Red team",
  "Blue team",
  "Yellow team",
  "Green team",
  "Power 100%",
  "Power 50%",
  "Full health",
  "Full ammo",
  "Add 1 mag",
  "Next Damage",
  "Health x2"
  
};

void draw(void) {  
  int shift=30;
  if (mode==0){
    u8g.drawFrame(15,23,98,22);//Draw current menu position frame if program in menu mode
    shift=18;
  }
  u8g.setPrintPos(shift, 18);
  u8g.print(str1);
  u8g.setPrintPos(shift, 40);
  u8g.print(str2);
  u8g.setPrintPos(shift, 62);
  u8g.print(str3);
  
}

void checkMenu(){
  if (curMenu==0){
    str1="";
  } else {
    str1=names[curMenu-1];
  }
  str2=names[curMenu];
  if (curMenu==arrCnt-1){
    str3="";
  } else {
    str3=names[curMenu+1];
  }
}

void checkShot(decode_results input) {
  int j,cnt;
  char packet[32];
  int packetLen;
  String packetStr, dmg, pid, col, dmgVal, colVal;
  int pidVal=0;
    
  ltoa(input.value, packet, 2); //convert to string with bits
  packetStr=packet;  
  
  if (packetStr.length() < 14 || (packetStr.length() < 24 && packetStr.length() > 14)) {//this is strange, but it works. Condition for shot
   mode=1; //Switch to display shot mode
   dmg=packetStr.substring(packetStr.length()-4,packetStr.length());
   col=packetStr.substring(packetStr.length()-6,packetStr.length()-4);
   pid=packetStr.substring(packetStr.length()-13,packetStr.length()-6);  

   if (dmg=="0000") {dmgVal="Dmg:  1";}
   if (dmg=="0001") {dmgVal="Dmg:  2";}
   if (dmg=="0010") {dmgVal="Dmg:  4";}
   if (dmg=="0011") {dmgVal="Dmg:  5";}
   if (dmg=="0100") {dmgVal="Dmg:  7";}
   if (dmg=="0101") {dmgVal="Dmg: 10";}
   if (dmg=="0110") {dmgVal="Dmg: 15";}
   if (dmg=="0111") {dmgVal="Dmg: 17";}
   if (dmg=="1000") {dmgVal="Dmg: 20";}
   if (dmg=="1001") {dmgVal="Dmg: 25";}
   if (dmg=="1010") {dmgVal="Dmg: 30";}
   if (dmg=="1011") {dmgVal="Dmg: 35";}
   if (dmg=="1100") {dmgVal="Dmg: 40";}
   if (dmg=="1101") {dmgVal="Dmg: 50";}
   if (dmg=="1110") {dmgVal="Dmg: 75";}
   if (dmg=="1111") {dmgVal="Dmg:100";}
      
   if (col=="00") {colVal="RED";}
   if (col=="01") {colVal="BLUE";}
   if (col=="10") {colVal="YELLOW";}
   if (col=="11") {colVal="GREEN";}

   
   for (j = 6; j >= 0; j--){       
    if (pid[j]=='1'){            
      pidVal=pidVal+(1<<(6-j));
    } 
   }
   
   str1=dmgVal;
   str2=colVal;
   str3="ID: "+String(pidVal);   
  } 
}

void setup(){
  Serial.begin(9600);
  irrecv.enableIRIn();    
  pinMode(btnB, INPUT);
  pinMode(btnM, INPUT);
  pinMode(btnN, INPUT);
  u8g.setFont(u8g_font_unifont);
  u8g.setColorIndex(0); //Set black color
  u8g.drawBox(0, 0, 128, 64);  //Fill display black
  u8g.setColorIndex(1); //Set white color
  mode=0; //Menu mode  
  curMenu=0; //Current index
}

void loop(){
  if (irrecv.decode(&results)) {    
    checkShot(results);
    irrecv.resume();
  }

  if (mode==0){ //If in menu mode - run function, that check position and draw items
    checkMenu();
  }

  curBtnB=digitalRead(btnB);
  curBtnM=digitalRead(btnM);
  curBtnN=digitalRead(btnN);

  if (curBtnM==HIGH && curBtnM!=prBtnM){ //Menu button press
    if (mode==0){ 
      irsend.sendSony(commands[curMenu], 24);            
      irrecv.enableIRIn();
    } else {
      mode=0; //Switch back to menu mode
    }
  }

  if (mode==0){ //Prev and next button. Menu mode only
    if (curBtnB==HIGH && curBtnB!=prBtnB){
      if (curMenu!=0){
        curMenu--;
      }
    }
    if (curBtnN==HIGH && curBtnN!=prBtnN){
      if (curMenu!=arrCnt-1){
        curMenu++;
      }
    }
  }

  //Remembering buttons states
  prBtnB=curBtnB;
  prBtnM=curBtnM;
  prBtnN=curBtnN;

  u8g.firstPage(); 
  do {
    draw();
  } while( u8g.nextPage() );

}


