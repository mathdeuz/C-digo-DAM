#include <Servo.h>
Servo servo1;

#include <SoftwareSerial.h>
SoftwareSerial MiBT(7, 8);

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 6
Adafruit_SSD1306 display(OLED_RESET);

#include <DS3231.h>
DS3231 rtc(SDA, SCL);
Time t;

#include <EEPROM.h>

int servo_pin = 4;
int stop_angle = 91;
int rotate_angle = 180;
int portions = 5;
int interval = 8; // Intervalo en minutos
unsigned long millis_before = 0;
unsigned long millis_now = 0;
int OLED_refresh_rate = 200;
int max_portions = 20;
int min_portions = 3;
int Hour, Minute;
int last_feed_hour = 1;
int next_feed_hour = 1;
bool feed_active = true;
int portion_delay = 500;
int opc = 0;
char c = '\0';
String words;

void setup() {
  feed_active = false;
  servo1.attach(servo_pin);
  servo1.write(stop_angle);
  Serial.begin(9600);
  MiBT.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(100);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();
  rtc.begin();
  last_feed_hour = EEPROM.read(2);
  next_feed_hour = EEPROM.read(3);
  millis_before = millis();
  millis_now = millis();
}

void loop() {
  while (MiBT.available()) {
    delay(500);
    c = MiBT.read();
    words += c;
  }

  opc = words.toInt();

  millis_now = millis();

  if (millis_now - millis_before > OLED_refresh_rate) {
    millis_before = millis();
    display.clearDisplay();
    display.setCursor(0, 0);
    String miPort = String(portions);
    display.print(miPort);
    display.print(" porciones");

    display.setCursor(90, 9);
    String miHour = String(Hour);
    display.print(miHour);
    display.print(":");
    String miMinute = String(Minute);
    display.print(miMinute);

    display.setCursor(0, 16);
    display.print("Cada ");
    String miInterval = String(interval);
    display.print(miInterval);
    display.print(" minutos");
    display.display();
  }

  t = rtc.getTime();
  Hour = t.hour;
  Minute = t.min;

  if (Hour * 60 + Minute == next_feed_hour) {
    feed_active = true;
    last_feed_hour = Hour * 60 + Minute;
    next_feed_hour = Hour * 60 + Minute + interval;

    if (next_feed_hour >= 24 * 60) {
      next_feed_hour = next_feed_hour - 24 * 60;
    }

    EEPROM.write(2, last_feed_hour);
    EEPROM.write(3, next_feed_hour);
  }

  if (feed_active) {
    int i = 0;
    while (i < portions) {
      servo1.write(rotate_angle);
      i++;
      display.clearDisplay();
      display.setCursor(38, 11);
      display.print("ALIMENTANDO");
      display.display();
      delay(portion_delay);
    }
    servo1.write(stop_angle);
    feed_active = false;
  }

  if (opc != 0) {
    if (opc >= 1 && opc <= 6) {
      switch (opc) {
        case 1:
          portions++;
          if (portions > max_portions) {
            portions = min_portions;
          }
          break;
        case 2:
          EEPROM.write(2, Hour * 60 + Minute);
          next_feed_hour = Hour * 60 + Minute + interval;
          if (next_feed_hour >= 24 * 60) {
            next_feed_hour = next_feed_hour - 24 * 60;
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
          if (interval > 10) {
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
            interval = 10; // Ajuste del límite a 10 minutos
          }
          break;
          case 6:
          // Manual_alimentacion
          feed_active = true;
          break;
      }
    }
    opc = 0;
    words = "";
  }
}