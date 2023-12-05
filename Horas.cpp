#include <Servo.h>  //Include the Servo Library
Servo servo1;   // "Servos" objects are created

#include <SoftwareSerial.h>
SoftwareSerial MiBT(7, 8);

#include <Adafruit_GFX.h>                       //Download here: https://electronoobs.com/eng_arduino_Adafruit_GFX.php
#include <Adafruit_SSD1306.h>                   //Download here: https://electronoobs.com/eng_arduino_ds3231.php  
#define OLED_RESET 6
Adafruit_SSD1306 display(OLED_RESET);

#include <DS3231.h>                             //Download here: https://electronoobs.com/eng_arduino_ds3231.php  
// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);
Time t;

#include <EEPROM.h>

int servo_pin = 4;
int stop_angle = 91;                //Please test values around 90 till the motor is not rotating anymore
int rotate_angle = 180;
int portions = 5;
int interval = 8;
unsigned int millis_before = 0;     //We use these to create the loop refresh rate
unsigned int millis_now = 0;
int OLED_refresh_rate = 200;
int max_portions = 20;
int min_portions = 3;
int Hour, Minute;
int last_feed_hour = 1;
int next_feed_hour = 1;
bool feed_active = true;
int portion_delay = 500;
int opc=0;
char c='\0';  
String words;     


void setup() {
  // put your setup code here, to run once:
  servo1.attach(servo_pin);  
  servo1.write(stop_angle);
  Serial.begin(9600);
  MiBT.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  delay(100);
  display.clearDisplay();                                                                                                                                                                     
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();
  rtc.begin();
  //rtc.setDOW(MONDAY);            // Set Day-of-Week to SUNDAY
  //rtc.setTime(16, 16, 20);         // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(11, 7, 2023);       // Set the date to January 1st, 2014
  last_feed_hour = EEPROM.read(2);  //We store the last feeding hour on address 2
  next_feed_hour = EEPROM.read(3);
  millis_before = millis();
  millis_now = millis();
}

void loop() {

while (MiBT.available()) {    // Read the value sent by the Serial Port
    delay(500);
    c  = MiBT.read();     // Read the characters
   // Serial.print("Caracter recibido: ");
   // Serial.println(c);
    words += c;              // Convert Characters to character string
  }

  opc=words.toInt();

  millis_now = millis();                                  //Get actual millis values each loop
  if(millis_now - millis_before > OLED_refresh_rate){     //Refresh rate of the OLED
    millis_before = millis();                             //Save "before value" for next loop
    display.clearDisplay();   
    display.setCursor(0,0);
    String miPort = String(portions);    
    display.print(miPort); 
    display.print(" porciones"); 
   
    display.setCursor(90,9);
    String miHour = String(Hour);      
    display.print(miHour); 
    display.print(":"); 
    String miMinute = String(Minute);      
    display.print(miMinute); 
   
    display.setCursor(0,16);
    display.print("Cada "); 
    String miInterval = String(interval);      
    display.print(miInterval); 
    display.print(" horas");
    display.display(); 

  }

  t = rtc.getTime();                          //Get time from DS3232
  Hour = t.hour;                              //Get the hour in 0h-23h format
  Minute = t.min;    
  Serial.println(Hour);                       //Print hour for debug

  if(Hour == next_feed_hour){                 //If the time is the "next_feed_hour",we activate feeding
    feed_active = true;
    last_feed_hour = Hour;
    next_feed_hour = Hour + interval;         //Increase next feeding time by the interval
    if(next_feed_hour >= 23){                 //if we pass 23 hours, we start back at 00 hours
      next_feed_hour = next_feed_hour - 24;   //That's why we substract 24 hours which is maximum
    }
    EEPROM.write(2, last_feed_hour);          //Write on memory when was the last feeding
    EEPROM.write(3, next_feed_hour);
    }
    
  if(feed_active){                            //If this is activem we rotate the motor
    int i = 0;
    while(i<portions){                        //Rotate the motor according to the portions value
      servo1.write(rotate_angle);
      i++;
      display.clearDisplay();   
      display.setCursor(38,11);      
      display.print("ALIMENTANDO");    
      display.display();      
      delay(portion_delay);                   //Delay for each portion is milliseconds
    }
    servo1.write(stop_angle);
    feed_active = false;                      //Set "feed_active" back to false
  }
if (opc!=0){     
    if(opc>=1 && opc<=6){
      switch(opc){
        case 1:
                  portions++;
                    if(portions > max_portions){          //Where "max_portions" is set above
                    portions = min_portions;
                    }
                  break;
        case 2:
                  EEPROM.write(2, Hour);
                  next_feed_hour = Hour + interval;
                    if(next_feed_hour >= 23){
                      next_feed_hour = next_feed_hour - 24;
                    }    
                  EEPROM.write(3, next_feed_hour);
				          display.clearDisplay();
					        display.setCursor(38, 11);
				          display.print("GUARDADO");
				          display.display();
				          delay(portion_delay);
                  break;
        case 3:
                  interval++;
                  if(interval > 23){
                    interval = 1;
                  }

                  break;
				case 4:
          // Reducir porciones, por ejemplo, en 1
          portions--;
          if (portions < min_portions) {
            portions = max_portions;
          }
          break;
        case 5:
          // Reducir intervalo, por ejemplo, en 1 minuto
          interval--;
          if (interval < 1) {
            interval = 23; // Ajuste del límite a 10 minutos
          }
          break;
          case 6:
          // Manual_alimentacion
          feed_active = true;
          break;
      }
    }
    opc=0;
    words = "";
  }  
}