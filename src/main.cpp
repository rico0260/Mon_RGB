/* 
 * Example sketch showing how to control an RGB Led Strip.
 * This example will remember the last rgb color set after power failure.
 * for : Home assistant
 * Auteur : Eric HANACEK
 * Modification : 11/2/2022 pour V2.3.2
*/
#include <Arduino.h>

#define SN "RGB Node"
#define SV "2.0"

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

// Enable repeater functionality for this node
#define MY_REPEATER_FEATURE

//Options: RF24_PA_MIN, RF24_PA_LOW, (RF24_PA_HIGH), RF24_PA_MAX
#define MY_RF24_PA_LEVEL RF24_PA_MAX

//#define MY_OTA_FIRMWARE_FEATURE

//uncomment this line to assign a static ID
//#define MY_NODE_ID AUTO 
#define MY_NODE_ID 31

//Reverse RGB - RBG
#define MY_RBG

//MY_RF24_CHANNEL par defaut 76
//Channels: 1 to 126 - 76 = Channel 77
//MY_RF24_CHANNEL (76)
#define MY_RF24_CHANNEL 81

//Define this to use the IRQ pin of the RF24 module
//#define MY_RF24_IRQ_PIN (2) 
//Define this to use the RF24 power pin (optional).
//#define MY_RF24_POWER_PIN (3)

//Placer les #define avant <Mysensors.h>
#include <MySensors.h>

#define CHILD_ID_RGB 0  // sensor number needed in the custom devices set up

#define RED  3  // Arduino PWM pin for Red
#ifdef MY_RBG   
  #define GREEN 5 // Arduino PWM pin for Green
  #define BLUE 6  // Arduino PWM pin for Blue
#else
  #define GREEN 6 // Arduino PWM pin for Green
  #define BLUE 5  // Arduino PWM pin for Blue
#endif

// Wait times
#define SHORT_WAIT 50
#define LONG_WAIT 100
#define LONG_WAIT2 2000

char stringRGB[7] = "000000";
//String StringRGB = "000000";
int on_off_status = 0;
int dimmerlevel = 100;
int RGB_pins[3] = {RED,GREEN,BLUE};
int RGB_values[3] = {0,0,0};


// initate and re-use to save memory.
MyMessage msgRGB(CHILD_ID_RGB, V_RGB);
MyMessage msgDIMMER(CHILD_ID_RGB, V_DIMMER);
//MyMessage msgLIGHT(CHILD_ID_RGB, V_LIGHT);
MyMessage msgSTATUS(CHILD_ID_RGB, V_STATUS);

void ModifierLED();

void before()
{
  // Optional method - for initialisations that needs to take place before MySensors transport has been setup (eg: SPI devices).
  // Set the rgb pins in output mode
  for (int i = 0; i<3; i++) {
    pinMode(RGB_pins[i], OUTPUT);  
  }
  //test des leds
  //Red
  analogWrite(RGB_pins[0], 255);
  analogWrite(RGB_pins[1], 0);
  analogWrite(RGB_pins[2], 0);
  wait(500);
  //Green
  analogWrite(RGB_pins[0], 0);
  analogWrite(RGB_pins[1], 255);
  analogWrite(RGB_pins[2], 0);
  wait(500);
  //Blue
  analogWrite(RGB_pins[0], 0);
  analogWrite(RGB_pins[1], 0);
  analogWrite(RGB_pins[2], 255);
  wait(500);
  //Eteindre
  analogWrite(RGB_pins[0], 0);
  analogWrite(RGB_pins[1], 0);
  analogWrite(RGB_pins[2], 0);

}

void presentation()
{
  Serial.print("===> Envoyer présentation pour noeud : ");
  Serial.println(MY_NODE_ID);
  
  char sNoeud[] = STR(MY_NODE_ID);

  // Send the sketch version information to the gateway and Controller
  Serial.println("=======> Envoyer SketchInfo");
  Serial.print(SN); Serial.print(" "); Serial.println(SV);
  sendSketchInfo(SN, SV );
  wait(LONG_WAIT2);

  // Register the sensor to gw
  Serial.println("=======> Présenter les capteurs");
  char sChild[25];
  strcpy(sChild, "myS ");
  strcat(sChild, sNoeud);
  strcat(sChild, " RGB");
  Serial.println(sChild);
  present(CHILD_ID_RGB, S_RGB_LIGHT, sChild);
  wait(LONG_WAIT2);  
}

void setup()
{

  // Set the rgb pins in output mode
  //valeur stocker dans eeprom
  Serial.println("=======> valeur stocker dans EEPROM");

  for (int i = 0; i<3; i++) {
    
    // Lecture des dernieres valeures dans eeprom 
    //RGB_values[i] = loadState(RGB_pins[i]);
    
    //analogWrite(RGB_pins[i], loadState(RGB_pins[i]));
    //analogWrite(RGB_pins[i], RGB_values[i]);
    
    // Afficher des infos de debugage
    if (i==0) {
      Serial.print("Rouge (R): " );
    }
    else if (i==1) {
      Serial.print("Vert (G): " );
    }
    else {
      Serial.print("Bleu (B): " );
    }
    //alternative
    // switch(i) {
    //   case 0:
    //     Serial.print("Rouge (R): " );
    //     break;
    //   case 1:
    //     Serial.print("Vert (G): " );
    //     break;
    //   default:
    //     Serial.print("Bleu (B): " );
    // }

    //valeur stocker dans eeprom
    Serial.print("EEPROM: "); Serial.println(loadState(RGB_pins[i]));
    //valeur envoyer
    //Serial.print(" Envoye: ");
    //Serial.println(RGB_values[i]);

  }

  // Lecture des dernieres valeures dans eeprom 
  //on_off_status = loadState(3);
  // Lecture des dernieres valeures dans eeprom 
  //dimmerlevel = loadState(4);

  //Demander la couleur actuelle pour le noeud
  Serial.println("==========> Requesting initial value from controller");
  #ifdef MY_DEBUG
     Serial.println("V_STATUS"); 
  #endif
  request( CHILD_ID_RGB, V_STATUS);
  wait(SHORT_WAIT);
  #ifdef MY_DEBUG
     Serial.println("V_RGB"); 
  #endif
  request( CHILD_ID_RGB, V_RGB);
  wait(SHORT_WAIT);
  #ifdef MY_DEBUG 
    Serial.println("V_DIMMER"); 
  #endif
  request( CHILD_ID_RGB, V_DIMMER);
  wait(SHORT_WAIT);
  //wait(LONG_WAIT, C_SET, V_RGB);

}

void loop()
{

  //Pour Home assistant
  static bool first_message_sent = false;
  if (!first_message_sent) {
    //send(msgPrefix.set("custom_lux"));  // Set custom unit.
    Serial.println("======> Sending initial value");
    //Serial.println(&RGB_values[0]);
    //La couleur
    Serial.println("Color Message");
    //send(msgRGB.set( "00FF00" ));
    send(msgRGB.set( stringRGB ));
    wait(LONG_WAIT2); //to check: is it needed
    //Le statut
    Serial.println("Status Message");
    send(msgSTATUS.set( on_off_status ));
    wait(LONG_WAIT2); //to check: is it needed
    //Dimmer
    Serial.println("Dimmer Message");
    send(msgDIMMER.set( dimmerlevel ));
    wait(LONG_WAIT2); //to check: is it needed

    //request(CHILD_ID_RGB, V_STATUS);
    //wait(2000, C_SET, V_STATUS);
    first_message_sent = true;
  }

}

void receive(const MyMessage &message) {
    
  /*Serial.print("Type message ");
  Serial.println(message.type);*/
  
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_RGB) {
    
    long number = (long) strtol( message.data, NULL, 16);

    // Save value
    RGB_values[0] = number >> 16;
    RGB_values[1] = number >> 8 & 0xFF;
    RGB_values[2] = number & 0xFF;

    strcpy(stringRGB, message.data);

    #ifdef MY_DEBUG
      Serial.println( "---> V_RGB: " );
      Serial.print("message in:  <-"); Serial.println(message.data);
      Serial.print("message out: ->"); Serial.println(stringRGB);
      //
      Serial.print("=> number: " ); Serial.println(number);
    #endif    

    //Changer etat des LEDs 
    ModifierLED();

    //Informe Home assistant de la couleur
    //wait(SHORT_WAIT);    
    send(msgRGB.set( stringRGB ));

  } else if (message.type == V_LIGHT || message.type == V_STATUS) {
    on_off_status = atoi(message.data);
    on_off_status = on_off_status == 1 ? 1 : 0;

    #ifdef MY_DEBUG 
      Serial.print( "V_LIGHT: <-" ); Serial.println(message.data);
      Serial.print( "V_LIGHT: ->" ); Serial.println(on_off_status);
    #endif

    //Changer etat des LEDs
    ModifierLED();    
    
    //Informe Home assistant de la couleur
    //wait(SHORT_WAIT);
    send(msgSTATUS.set( on_off_status ));

  } else if (message.type == V_DIMMER || message.type == V_PERCENTAGE) {
    dimmerlevel = atoi(message.data);
    dimmerlevel = dimmerlevel > 100 ? 100 : dimmerlevel;
    dimmerlevel = dimmerlevel < 0 ? 0 : dimmerlevel;
    
    #ifdef MY_DEBUG 
      Serial.print( "V_DIMMER: <-" ); Serial.println(message.data);
      Serial.print( "V_DIMMER: ->" ); Serial.println(dimmerlevel);
    #endif 

    //Changer etat des LEDs
    ModifierLED();    
    
    //Informe Home assistant de la couleur
    //wait(SHORT_WAIT);    
    send(msgDIMMER.set( dimmerlevel ));

  }

}

void ModifierLED() {
int setRGB[3] = {0,0,0};

  setRGB[0] = on_off_status * (int)(RGB_values[0] * dimmerlevel/100.0);
  setRGB[1] = on_off_status * (int)(RGB_values[1] * dimmerlevel/100.0);
  setRGB[2] = on_off_status * (int)(RGB_values[2] * dimmerlevel/100.0);

  for (int i = 0; i<3; i++) {
    //Changer etat des LEDs 
    analogWrite(RGB_pins[i],setRGB[i]);  
    // Enregistrer dans eeprom
    //saveState(RGB_pins[i], RGB_values[i]);
  }
  //save on/off
  //saveState(3, on_off_status);
  //save dimmer
  //saveState(4, dimmerlevel);
  #ifdef MY_DEBUG 
    Serial.println( "-----> Modification RGB: " ); 
    Serial.print( "     on_off_status: " ); Serial.println(on_off_status);
    Serial.print( "     dimmerlevel: " ); Serial.println(dimmerlevel);
    Serial.print( "     setRGB: " ); Serial.print(setRGB[0]); 
    Serial.print("-"); Serial.print(setRGB[1]);
    Serial.print("-"); Serial.println(setRGB[2]);
  #endif 


}