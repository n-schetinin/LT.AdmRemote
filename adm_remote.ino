#include <U8glib.h>

#include <IRremote.h>
#include <IRremoteInt.h>
#include <LaserTag_IRSend.h>
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); //display init

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

int voltage, voltCount, vc; //variables for battery indicator
unsigned long accumTime; //time for battery check


int mode; //working mode (0 - menu, 1 - displaying shot)

IRrecv irrecv(2); //IR recieve on pin 2
decode_results results;
LTSend LT(12); // IR send on pin 12

//Display strings
String str1 = "";
String str2 = "";
String str3 = "";

int curMenu; //Current menu index

const int arrCnt = 14; //menu items count

long commands[arrCnt] = { //commands values
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
  0x8322E8,
  0xA032E8

};

String names[arrCnt] = { //commands screen names
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
  "Health x2",
  "Radiation"
};

void draw(void) { //drawing display content
  int shift = 30; //shift (from left border) for centering shot info


  if (mode == 0) {
    u8g.drawFrame(15, 25, 98, 20); //Draw current menu position frame in menu mode
    shift = 18; // shift for menu
  }
  //draw display strings
  u8g.setPrintPos(shift, 20);
  u8g.print(str1);
  u8g.setPrintPos(shift, 41);
  u8g.print(str2);
  u8g.setPrintPos(shift, 62);
  u8g.print(str3);

  
  u8g.setColorIndex(0);//set black color
  u8g.drawBox(109, 0, 18, 8);  //fill indicator place black
  u8g.setColorIndex(1); //set white color
  u8g.drawFrame(110, 0, 18, 7); //Battery indicator frame

  //drawing indicator pixels
  for (vc = 0; vc <= voltCount; vc++) {
    u8g.drawBox(112 + (vc - 1) * 3, 2, 2, 3);
  }
}

void checkMenu() { //creating display strings for menu
  if (curMenu == 0) {
    str1 = "";
  } else {
    str1 = names[curMenu - 1];
  }
  str2 = names[curMenu];
  if (curMenu == arrCnt - 1) {
    str3 = "";
  } else {
    str3 = names[curMenu + 1];
  }
}

void checkShot(decode_results input) { //checking signal for shot or not
  int j, cnt;
  char packet[32];
  char packet16[8];
  int packetLen;
  String packetStr, packetStr16, dmg, pid, col, dmgVal, colVal;
  int pidVal = 0;  

  ltoa(input.value, packet, 2); //convert to char array with bits
  ltoa(input.value, packet16, 16); //convert to char array with hex
  //converting arrays to strings
  packetStr = packet; 
  packetStr16 = packet16;


  if (packetStr.length() == 13 ) {//Condition for shot. Normally 14 bits, but first is unsigned zero due the protocol
    mode = 1; //Switch to display shot mode
    dmg = packetStr.substring(packetStr.length() - 4, packetStr.length()); //substring for damage value
    col = packetStr.substring(packetStr.length() - 6, packetStr.length() - 4); //substring for color value
    pid = packetStr.substring(packetStr.length() - 13, packetStr.length() - 6); //substring for ID

    //converting damage substring to value
    if (dmg == "0000") {
      dmgVal = "Dmg:  1";
    }
    if (dmg == "0001") {
      dmgVal = "Dmg:  2";
    }
    if (dmg == "0010") {
      dmgVal = "Dmg:  4";
    }
    if (dmg == "0011") {
      dmgVal = "Dmg:  5";
    }
    if (dmg == "0100") {
      dmgVal = "Dmg:  7";
    }
    if (dmg == "0101") {
      dmgVal = "Dmg: 10";
    }
    if (dmg == "0110") {
      dmgVal = "Dmg: 15";
    }
    if (dmg == "0111") {
      dmgVal = "Dmg: 17";
    }
    if (dmg == "1000") {
      dmgVal = "Dmg: 20";
    }
    if (dmg == "1001") {
      dmgVal = "Dmg: 25";
    }
    if (dmg == "1010") {
      dmgVal = "Dmg: 30";
    }
    if (dmg == "1011") {
      dmgVal = "Dmg: 35";
    }
    if (dmg == "1100") {
      dmgVal = "Dmg: 40";
    }
    if (dmg == "1101") {
      dmgVal = "Dmg: 50";
    }
    if (dmg == "1110") {
      dmgVal = "Dmg: 75";
    }
    if (dmg == "1111") {
      dmgVal = "Dmg:100";
    }

    //converting color substring to value
    if (col == "00") {
      colVal = "RED";
    }
    if (col == "01") {
      colVal = "BLUE";
    }
    if (col == "10") {
      colVal = "YELLOW";
    }
    if (col == "11") {
      colVal = "GREEN";
    }

    //converting ID to value
    for (j = 6; j >= 0; j--) {
      if (pid[j] == '1') {
        pidVal = pidVal + (1 << (6 - j));
      }
    }

    //setting display strings
    str1 = dmgVal;
    str2 = colVal;
    str3 = "ID: " + String(pidVal);
  } else { //if not a shot - drawing hex value of received signal
    mode = 2;
    str1 = "Not a shot";
    str2 = "#" + packetStr16;
    str3 = "";
  }

}

void setup() {  
  irrecv.enableIRIn(); //inabling receiving
  pinMode(btnB, INPUT); 
  pinMode(btnM, INPUT);
  pinMode(btnN, INPUT);
  
  u8g.setFont(u8g_font_unifont); //set font for display
  u8g.setColorIndex(0); //Set black color
  u8g.drawBox(0, 0, 128, 64);  //Fill display black
  u8g.setColorIndex(1); //Set white color
  mode = 0; //Menu mode
  curMenu = 0; //Current index
  accumTime = 0; //millis() time for next voltage check
}

void loop() {
  //Counting battery pixels
  if (millis() > accumTime) {
    voltage = analogRead(0); //Checking accum voltage
    if (voltage >= 860) {
      voltCount = 5;
    }
    if (voltage < 860) {
      voltCount = 4;
    }
    if (voltage < 819) {
      voltCount = 3;
    }
    if (voltage < 778) {
      voltCount = 2;
    }
    if (voltage < 737) {
      voltCount = 1;
    }
    if (voltage < 696) {
      voltCount = 0;
    }
    accumTime=accumTime+60000;
  }


  if (irrecv.decode(&results)) { // if received signsl
    checkShot(results); // checking is shot or not
    irrecv.resume(); //resuming receive
  }

  if (mode == 0) { //If in menu mode - run function, that check position and draw items
    checkMenu(); //drawing menu
  }

  //getting current button states
  curBtnB = digitalRead(btnB);
  curBtnM = digitalRead(btnM);
  curBtnN = digitalRead(btnN);


  if (curBtnM == HIGH && curBtnM != prBtnM) { //Menu button pressed
    if (mode == 0) { //if in menu mode, sending LT command    
      LT.Command(commands[curMenu], 24);
      irrecv.enableIRIn(); //enabling receiving after send
    } else {
      mode = 0; //Switch back to menu mode
    }
  }

  if (mode == 0) { //Prev and next button. Menu mode only
    if (curBtnB == HIGH && curBtnB != prBtnB) {
      if (curMenu != 0) {
        curMenu--;
      }
    }
    if (curBtnN == HIGH && curBtnN != prBtnN) {
      if (curMenu != arrCnt - 1) {
        curMenu++;
      }
    }
  }

  //Remembering buttons states
  prBtnB = curBtnB;
  prBtnM = curBtnM;
  prBtnN = curBtnN;

  u8g.firstPage(); //redrawing display
  do {
    draw();
  } while ( u8g.nextPage() );

}
