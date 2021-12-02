/*  Plush Toys Controller System
    ilyas9461 30.11.2021
*/

//#include "note_strokes.h"
#include "pin_mp3_defines.h"
#include <DFPlayer_Mini_Mp3.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Wire.h>

SoftwareSerial dfSerial(DF_PLAYER_ARDU_RX_PIN, DF_PLAYER_ARDU_TX_PIN); // RX, TX ARDU side pins.

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

int engine_speed = 250;    // max engine speed, 0-255
int engine_ramp_speed = 0; // engine soft start speed

int menuCount = 0;  // OK btn press count and call the menu function.
int menuChoise = 0; // menu items.

/* Operational Flags*/
bool engine_dir_forward = true; // forward direction switch status
bool last_engine_dir = true;    // last direction find
bool over_current = false;
bool is_coin = false;
bool is_pressed_gas_pedal = false;

/* Discrete time transactions*/
long time_vi_show = 0;      //  Acculumator Current and voltage show
long time_i_mesurement = 0; //  Moving average measurement for current
long time_ramp_speed = 0;
long time_ramp_stop = 0;
long time_game = 0;
long time_low_voltage = 0;
unsigned long game_timer = 0lu;

unsigned int token_count = 0;
uint8_t ee_game_time = 3;

float current = 0.0;
float Iaku = 0.0;

int low_voltage_count = ACC_V_READ_DELAY;
bool is_say_low_voltage = false;

byte current_mesurement_count = 0;

byte mp3Count = 0;
uint8_t volumeSet = 20;

unsigned int front_sens_stat = 1;

float max_current = 4.12;

void setup()
{
  // Serial.begin(9600); // Initialize Serial to log output

  dfSerial.begin(9600);
  init_pin();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("PELUS OYUNCAKLAR");
  delay(1500);
  lcd.noBacklight();
  delay(500);

  rgb_initial();
  // r_led_on();
  // delay(1000);
  // g_led_on();
  // delay(1000);
  // b_led_on();
  // delay(1000);

  rgb_off();

  mp3_set_serial(dfSerial); // set softwareSerial for DFPlayer-mini mp3 module
  delay(100);               // wait 1ms for mp3 module to set volume
  mp3_set_volume(25);
  delay(100); // wait 1ms for mp3 module to set volume
  mp3_stop();
  delay(250);
  mp3_play_physical(MP3_BEGIN); // mp3_play(MP3_BEGIN);
  delay(250);

  tone(BUZZER_PIN, 500, 50); // tone(pin, frequency, duration)
  delay(3000);

  ee_initial_stat();

  time_vi_show = time_i_mesurement = time_ramp_speed = time_ramp_stop = time_low_voltage = millis();
}

void loop()
{
  if (!digitalRead(DF_BUSSY_PIN)) // dfbusy pin inverted with ULN2003 open collector driver ic.
                                  // High -> bussy(play), low -> NULL, NO play
  {
    // mp3_stop();
    // delay(250);
    // mp3_play(mp3_adele_Rolling + mp3Count);
    // delay(250);
    // mp3Count++;
    // if (mp3Count >= 6)
    //   mp3Count = 0;

    if (is_coin)
    {
      mp3_play(MP3_BABY_SHARK);
      delay(250);
    }
  }

  movingAverageMesurement();

  if (!digitalRead(JTN_PIN)) // Toy start if takes coin .
  {
    front_sens_stat = false;
    tone(BUZZER_PIN, 1500, 50); // tone(pin, frequency, duration)
    delay(100);

    is_coin = true;
    mp3_stop();
    delay(100);
    mp3_set_volume(volumeSet);
    delay(100);
    mp3_play(MP3_CAR_START);
    delay(3000);
    mp3_play(MP3_HOCKEY);
    delay(3100);
    mp3_play(MP3_BABY_SHARK);
    delay(100);

    token_count++;
    EEPROM.put(EE_ADR_COUNT_TOKEN, token_count);

    front_sens_stat = true;

    time_game = millis();
  }

  if ((millis() - time_game > game_timer) && is_coin) // Toy playtime in minutes.  //(ee_game_time * 1000 * 60)
  {
    mp3_stop();
    engineStopDelay();
    mp3_play(MP3_GAME_OVER);
    delay(5000);
    mp3_play(MP3_APPLAUSE);
    delay(3000);
    is_coin = false;
  }

  if (is_coin)
  {
    if (!digitalRead(GAS_PEDAL_PIN))
    {
      is_pressed_gas_pedal = true;
      motorRun();
    }
    else
    {
      is_pressed_gas_pedal = false;    
      engineRampStop();//engineStop();
    }
  }

  if (!digitalRead(MZ80_PIN) && front_sens_stat)
  {
    if (engine_ramp_speed > 0)
      // engineStop();
      engineStopDelay();
    // tone(BUZZER_PIN, 2700, 100); //tone(pin, frequency, duration)
    // delay(100);
    mp3_stop();

    delay(100);
    mp3_play_physical(13);
    delay(1500);
    if (low_voltage_count > 0 && low_voltage_count <= 5)
      low_voltage_count--;
  }

  if (!digitalRead(DIR_SW_PIN))
  {
    engine_dir_forward = false; // back direction
    if (last_engine_dir != engine_dir_forward && engine_ramp_speed > 0)
    {
      is_pressed_gas_pedal = false;
      engine_dir_forward = true; // forward direction
      while (engine_ramp_speed > 0)
      {
        engineRampStop();
        if (engine_ramp_speed < 10)
          break;
      }
    }
    engine_dir_forward = false; // back direction
  }
  else
  {
    engine_dir_forward = true; //  forward direction
    if (last_engine_dir != engine_dir_forward && engine_ramp_speed > 0)
    {
      is_pressed_gas_pedal = false;
      engine_dir_forward = false; // back direction stop
      while (engine_ramp_speed > 0)
      {
        engineRampStop();
        if (engine_ramp_speed < 10)
          break;
      }
    }
    engine_dir_forward = true;
  }

  last_engine_dir = engine_dir_forward;

  if (!digitalRead(BTN_OK_PIN))
  {
    tone(BUZZER_PIN, 450, 50); // tone(pin, frequency, duration)
    delay(100);
    menuCount++;
    if (menuCount >= 10)
    {
      menuCount = 0;
      lcdMenu();
    }

    showMesurements();
    time_vi_show = millis();
  }

  if (millis() - time_vi_show > 5000)
  {
    lcd.noBacklight();
    menuCount = 0;
    // time_vi_show = millis();
  }

  if (millis() - time_low_voltage > 60000lu && is_say_low_voltage)
  {
    low_voltage_count++;

    if (low_voltage_count > 250)
    {
      // low_voltage_count = 0;
      is_say_low_voltage = false;
    }

    if (low_voltage_count > 0)
    {
      mp3_stop();
      delay(100);
      mp3_set_volume(29);
      delay(100);
      mp3_play_physical(MP3_ACC_VOLTAGE);
      delay(1000);
    }

    time_low_voltage = millis();
  }

} // loop

void motorRun()
{

  if (!over_current)
  {
    if (engine_dir_forward)
    {
      engineGoForward(engine_speed);
    }
    else
    {
      engineGoBack(engine_speed); //
    }
  }
}

void lcdMenu()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("      MENU");
  lcd.setCursor(0, 1);
  // lcd.print(" <     <>     >");

  delay(1000);
  bool btn = true;
  while (btn)
  {
    // menuItemsIncDec();
    if (!digitalRead(BTN_RIGHT_PIN))
    {
      delay(BTN_DELAY);
      menuChoise++;
      if (menuChoise >= 9)
        menuChoise = 1;
    }

    if (!digitalRead(BTN_LEFT_PIN))
    {
      delay(BTN_DELAY);
      menuChoise--;
      if (menuChoise <= 0)
        menuChoise = 8;
    }

    if (menuChoise == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print("1-OYUN SURE AYAR");
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");

      if (!digitalRead(BTN_OK_PIN))
      {
        delay(BTN_DELAY);
        game_time_setting();
      }
    }

    if (menuChoise == 2)
    {
      lcd.setCursor(0, 0);
      lcd.print("2-HIZ AYAR      ");
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");

      if (!digitalRead(BTN_OK_PIN))
      {
        delay(BTN_DELAY);
        speed_setting();
      }
    }

    if (menuChoise == 3)
    {
      lcd.setCursor(0, 0);
      lcd.print("3-SES AYAR      ");
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");

      if (!digitalRead(BTN_OK_PIN))
      {
        delay(BTN_DELAY);
        volume_setting();
      }
    }

    if (menuChoise == 4)
    {
      lcd.setCursor(0, 0);
      lcd.print("4-DURMA SURESI  ");
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");

      if (!digitalRead(BTN_OK_PIN))
      {
        delay(BTN_DELAY);
        // stop_time_setting();
      }
    }

    if (menuChoise == 5)
    {
      lcd.setCursor(0, 0);
      lcd.print("5-HIZLANMA SURE ");
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");

      if (!digitalRead(BTN_OK_PIN))
      {
        delay(BTN_DELAY);
        // speed_time_setting();
      }
    }

    if (menuChoise == 6)
    {
      lcd.setCursor(0, 0);
      lcd.print("6-MAX. AKIM     ");
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");

      if (!digitalRead(BTN_OK_PIN))
      {
        delay(BTN_DELAY);
        max_current_setting();
      }
    }

    if (menuChoise == 7)
    {
      lcd.setCursor(0, 0);
      lcd.print("7-On SENS ON/OFF");
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");

      if (!digitalRead(BTN_OK_PIN))
      {
        delay(BTN_DELAY);
        front_sens_on_off_setting();
      }
    }

    if (menuChoise == 8)
    {
      lcd.setCursor(0, 0);
      lcd.print("8-CIKIS         ");
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");

      if (!digitalRead(BTN_OK_PIN))
      {
        delay(BTN_DELAY);
        break;
        // btn=false;
      }
    }

  } // while(1)

} //

void max_current_setting()
{
  // unsigned int int_c = EEPROM.get(EE_ADR_MAX_I_INT_VALUE);
  // unsigned int float_c = EEPROM.get(EE_ADR_MAX_I_FLOAT_VALUE);
  int int_c;
  int float_c;
  EEPROM.get(EE_ADR_MAX_I_INT_VALUE,int_c);
  EEPROM.get(EE_ADR_MAX_I_FLOAT_VALUE, float_c);

  // char buffer[25];
  // String buffer="";

  if (int_c == 0xFFFF && float_c == 0xFFFF)
  {
    int_c = 4;
    float_c = 2;
  }

  if (int_c >10) int_c = 4;
  if(float_c>9) float_c = 0;

  delay(1000);
  lcd.clear();

  do
  {
    max_current = (float)((float)int_c + (float)float_c / 10.0);
    // sprintf(buffer, "Imax: %f A", max_current);
    // buffer="Imax : "+String(max_current)+" A";

    lcd.setCursor(0, 0);
    lcd.print(String(max_current)); //
    lcd.setCursor(0, 1);
    lcd.print("<      <>      >");

    if (!digitalRead(BTN_RIGHT_PIN))
    {
      delay(BTN_DELAY);
      float_c++;
      if (float_c > 9)
      {
        int_c++;
        float_c = 0;
      }
      lcd.clear();
    }

    if (!digitalRead(BTN_LEFT_PIN))
    {
      delay(BTN_DELAY);
      float_c--;
      if (float_c < 0)
      {
        int_c--;
        float_c = 9;
      }

      lcd.clear();
    }

  } while (digitalRead(BTN_OK_PIN));

  delay(BTN_DELAY);
  // EEPROM.write(EE_ADR_MAX_I_INT_VALUE, int_c);
  // EEPROM.write(EE_ADR_MAX_I_FLOAT_VALUE, float_c);

  EEPROM.put(EE_ADR_MAX_I_INT_VALUE, int_c);
  EEPROM.put(EE_ADR_MAX_I_FLOAT_VALUE, float_c);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kaydedildi...");
  delay(1500);
}

void game_time_setting()
{
  int game_time = EEPROM.read(EE_ADR_GAME_TIME);
  if (game_time == 255)
    game_time = 1;
  delay(1000);
  lcd.clear();

  do
  {
    lcd.setCursor(0, 0);
    lcd.print("Oyun Sure:" + String(game_time) + " Dk");
    lcd.setCursor(0, 1);
    lcd.print("<      <>      >");

    if (!digitalRead(BTN_RIGHT_PIN))
    {
      delay(BTN_DELAY - 200);
      game_time++;
      if (game_time >= 30)
        game_time = 1;
      lcd.clear();
    }
    if (!digitalRead(BTN_LEFT_PIN))
    {
      delay(BTN_DELAY - 200);
      game_time--;
      if (game_time <= 1)
        game_time = 1;
      lcd.clear();
    }

  } while (digitalRead(BTN_OK_PIN));

  delay(BTN_DELAY);
  EEPROM.put(EE_ADR_GAME_TIME, game_time);
  ee_game_time = game_time;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kaydedildi...");
  delay(1500);
}

void speed_setting()
{
  int speed = EEPROM.read(EE_ADR_SPEED);
  if (speed == 255)
    speed = 200;
  char sSpeed[10];

  delay(1000);
  lcd.clear();

  do
  {
    sprintf(sSpeed, "HIZ: %u", speed);
    lcd.setCursor(0, 0);
    lcd.print(sSpeed); //
    lcd.setCursor(0, 1);
    lcd.print("<      <>      >");

    if (!digitalRead(BTN_RIGHT_PIN))
    {
      delay(BTN_DELAY);
      speed++;
      if (speed >= 250)
        speed = 250;
      lcd.clear();
    }

    if (!digitalRead(BTN_LEFT_PIN))
    {
      delay(BTN_DELAY);
      speed--;
      if (speed <= 1)
        speed = 1;
      lcd.clear();
    }

  } while (digitalRead(BTN_OK_PIN));

  delay(BTN_DELAY);
  // EEPROM.write(EE_ADR_SPEED, speed);
  EEPROM.put(EE_ADR_SPEED, speed);
  engine_speed = speed;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kaydedildi...");
  delay(1500);
}

void volume_setting()
{
  char buffer[10];

  delay(1000);
  lcd.clear();

  do
  {
    sprintf(buffer, "SES: %u", volumeSet);
    lcd.setCursor(0, 0);
    lcd.print(buffer); //
    lcd.setCursor(0, 1);
    lcd.print("<      <>      >");

    if (!digitalRead(BTN_RIGHT_PIN))
    {
      delay(BTN_DELAY);
      volumeSet++;
      if (volumeSet >= 30)
        volumeSet = 30;
      lcd.clear();
    }

    if (!digitalRead(BTN_LEFT_PIN))
    {
      delay(BTN_DELAY);
      volumeSet--;
      if (volumeSet <= 1)
        volumeSet = 1;
      lcd.clear();
    }

  } while (digitalRead(BTN_OK_PIN));

  delay(BTN_DELAY);
  EEPROM.put(EE_ADR_VOLUME, volumeSet);

  mp3_set_volume(volumeSet);
  delay(100);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kaydedildi...");
  delay(1500);
}

void front_sens_on_off_setting()
{
  byte sens_stat = 1; // EEPROM.read(EE_ADR_FRONT_SENS_STAT);

  if (sens_stat == 255)
    sens_stat = 1;

  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("<      <>      >");

  do
  {
    if (!digitalRead(BTN_RIGHT_PIN))
    {
      delay(BTN_DELAY);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ON SENS. ON"); //
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");
      sens_stat = 1;
    }

    if (!digitalRead(BTN_LEFT_PIN))
    {
      delay(BTN_DELAY);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ON SENS. OFF"); //
      lcd.setCursor(0, 1);
      lcd.print("<      <>      >");
      sens_stat = 0;
    }

  } while (digitalRead(BTN_OK_PIN));

  delay(BTN_DELAY);
  EEPROM.put(EE_ADR_FRONT_SENS_STAT, (unsigned int)sens_stat);
  front_sens_stat = sens_stat;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kaydedildi...");
  delay(1500);
}

void showMesurements()
{
  float vAku = voltageAccMeasurement();
  // float Iaku = currentAccMeasurement();

  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Vaku=" + String(vAku) + " V");
  lcd.setCursor(0, 1);
  lcd.print("Iaku=" + String(Iaku) + " A");
}

void movingAverageMesurement()
{

  if (millis() - time_i_mesurement >= 50)
  {
    float instantaneous_current = currentAccMeasurement();
    float vAku = voltageAccMeasurement();

    current += instantaneous_current;
    current_mesurement_count++;

    Iaku = current / current_mesurement_count;

    if (Iaku > max_current)
    {
      engineStop();
      mp3_stop();
      delay(100);
      over_current = true;
      tone(BUZZER_PIN, 100, 100); // tone(pin, frequency, duration)
    }
    else if (Iaku < 2.5)
    {
      over_current = false;
    }

    if (vAku < 12.0 && Iaku < 1.0)
    {
      if (low_voltage_count <= 250 && !is_coin)
        is_say_low_voltage = true;
      else
        is_say_low_voltage = false;
    }

    if (vAku > 12.0)
    {
      is_say_low_voltage = false;
      low_voltage_count = ACC_V_READ_DELAY;
    }

    if (current_mesurement_count >= 20)
    {
      current_mesurement_count = 0;
      Iaku = current / 20;
      current = 0.0;
    }

    time_i_mesurement = millis();
  }
}

float voltageAccMeasurement()
{
  const float r1 = 100.0; // K
  const float r2 = 22.0;

  unsigned int anAku = 0;
  for (int i = 0; i < 50; i++)
  {
    anAku += analogRead(VAKU_PIN);
  }

  anAku /= 50;

  float vAnAKu = (5.0 / 1023.0) * (float)anAku;

  float vAku = ((r1 + r2) / r2) * vAnAKu; // (122/22)*anAku

  return (float)vAku;
}

float currentAccMeasurement()
{
  // ACS711EX Current Sensor Carrier -31A to +31A
  unsigned long anIaku = 0;

  for (int i = 0; i < 25; i++)
  {
    anIaku += analogRead(IAKU_PIN);
  }

  anIaku /= 25;

  //ACS711EX
  float Viaku = (5.0 / 1023.0) * (float)anIaku;
  // float iAku = 73.3 * (Viaku / 5.0) - 36.7;
  float iAku = ((Viaku * 1000.0) - 2500.0) / 75.0; // Vcc=3.3V iken 45.0 , Vcc=5V 68.0 mV/A

  // Serial.println(iAku);

  return iAku;
} // end current sensor

void ee_initial_stat()
{
  lcd.backlight();

  EEPROM.get((int)EE_ADR_GAME_TIME, ee_game_time);
  // ee_game_time=EEPROM.read(EE_ADR_GAME_TIME);
  if (ee_game_time == 255)
    ee_game_time = 3;

  game_timer = (unsigned long)ee_game_time * 60lu * 1000lu;

  lcd.clear();
  lcd.setCursor(0, 0);
  // lcd.print("OYUN SURESI :");
  lcd.print("OS:" + String(game_timer));
  lcd.setCursor(0, 1);
  lcd.print("  " + String(ee_game_time) + " DK");
  delay(2000);

  EEPROM.get(EE_ADR_VOLUME, volumeSet);
  // volumeSet=EEPROM.read(EE_ADR_VOLUME);
  if (volumeSet == 255)
    volumeSet = 20;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SES :");
  lcd.setCursor(0, 1);
  lcd.print("  " + String(volumeSet));
  delay(2000);

  // engine_speed = EEPROM.read(EE_ADR_SPEED);
  EEPROM.get((int)EE_ADR_SPEED, engine_speed);
  if (engine_speed == 255)
    engine_speed = 200;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MAX. HIZ :");         
  lcd.setCursor(0, 1);
  lcd.print("  " + String(engine_speed));

  delay(2000);

  // front_sens_stat = EEPROM.read(EE_ADR_FRONT_SENS_STAT);
  EEPROM.get((int)EE_ADR_FRONT_SENS_STAT, front_sens_stat);
  if (front_sens_stat == 255)
    front_sens_stat = 1;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ON SENSOR :");
  lcd.setCursor(0, 1);
  lcd.print("  " + String(front_sens_stat));
  delay(2000);

  // int int_c = EEPROM.read(EE_ADR_MAX_I_INT_VALUE);
  // int float_c = EEPROM.read(EE_ADR_MAX_I_FLOAT_VALUE);

  unsigned int int_c = 4;
  EEPROM.get((int)EE_ADR_MAX_I_INT_VALUE, int_c);
  unsigned int float_c = 2;
  EEPROM.get((int)EE_ADR_MAX_I_FLOAT_VALUE, float_c);

  if (int_c == 255 && float_c == 255)
  {
    int_c = 4;
    float_c = 2;
  }
  max_current = (float)((float)int_c + (float)float_c / 10.0);

  if(max_current<0.0) max_current=4.00;
  if(max_current>20) max_current=10;

  lcd.clear();
  lcd.setCursor(0, 0);
  // lcd.print(String(int_c)+","+String(float_c));
  lcd.print("MAX. AKIM :");
  lcd.setCursor(0, 1);
  lcd.print("   " + String(max_current) + " A");
  delay(2000);

  EEPROM.get(EE_ADR_COUNT_TOKEN, token_count);
  if (token_count == 0xFFFF)
    token_count = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("KONTUR SYS. :");
  lcd.setCursor(0, 1);
  lcd.print(String(token_count));
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ilyas9461");
  lcd.setCursor(0, 1);
  lcd.print("01.11.2021");

} //

void init_pin()
{
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(MOTOR_RPWM_PIN, OUTPUT);
  pinMode(MOTOR_LPWM_PIN, OUTPUT);

  pinMode(DF_BUSSY_PIN, INPUT);

  pinMode(VAKU_PIN, INPUT);
  pinMode(IAKU_PIN, INPUT);

  pinMode(JTN_PIN, INPUT_PULLUP);
  pinMode(DIR_SW_PIN, INPUT_PULLUP);

  pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
  pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BTN_OK_PIN, INPUT_PULLUP);

  pinMode(GAS_PEDAL_PIN, INPUT_PULLUP);
  pinMode(MZ80_PIN, INPUT_PULLUP);

  digitalWrite(MOTOR_RPWM_PIN, LOW);
  digitalWrite(MOTOR_LPWM_PIN, LOW);
}

void engineGoForward(int hiz)
{
  if (millis() - time_ramp_speed > 15 && is_pressed_gas_pedal) // 5000mS/200=25mS
  {
    engine_ramp_speed++;
    if (engine_ramp_speed > 255)
      engine_ramp_speed = 255;

    if (engine_ramp_speed > hiz)
      engine_ramp_speed = hiz;

    time_ramp_speed = millis();
  }

  if (is_pressed_gas_pedal)
  {

    analogWrite(MOTOR_LPWM_PIN, 0);
    // digitalWrite(MOTOR_LPWM_PIN, LOW);
    analogWrite(MOTOR_RPWM_PIN, engine_ramp_speed);
  }
}

void engineGoBack(byte hiz)
{
  if (millis() - time_ramp_speed > 15 && is_pressed_gas_pedal) // 10000mS/200=50mS
  {
    engine_ramp_speed++;
    if (engine_ramp_speed > 255)
      engine_ramp_speed = 255;

    if (engine_ramp_speed > hiz)
      engine_ramp_speed = hiz;

    time_ramp_speed = millis();
  }

  if (is_pressed_gas_pedal)
  {
    analogWrite(MOTOR_LPWM_PIN, (-255) + engine_ramp_speed);
    analogWrite(MOTOR_RPWM_PIN, 0);
    // digitalWrite(MOTOR_RPWM_PIN, LOW);
  }
}

void engineRampStop()
{
  if (millis() - time_ramp_stop > 5 && !is_pressed_gas_pedal) // 3000ms 200 hız için
  {
    if (engine_ramp_speed > 0)
      engine_ramp_speed--;

    if (engine_ramp_speed < 10)
    {
      engine_ramp_speed = 0;
      engineStop();
    }

    time_ramp_stop = millis();
  }

  if (!is_pressed_gas_pedal && engine_dir_forward) // ileri yon
  {
    analogWrite(MOTOR_LPWM_PIN, 0);
    analogWrite(MOTOR_RPWM_PIN, engine_ramp_speed);
  }

  if (!is_pressed_gas_pedal && !engine_dir_forward) // geri yon
  {
    analogWrite(MOTOR_LPWM_PIN, engine_ramp_speed);
    analogWrite(MOTOR_RPWM_PIN, 0);
    digitalWrite(MOTOR_RPWM_PIN, LOW);
  }
}

void engineStopDelay()
{
  for (int i = 0; i < engine_speed; i++)
  {
    if (engine_dir_forward) // ileri yon
    {
      analogWrite(MOTOR_LPWM_PIN, 0);
      analogWrite(MOTOR_RPWM_PIN, engine_ramp_speed--);
    }
    else // geri yon
    {
      analogWrite(MOTOR_LPWM_PIN, engine_ramp_speed--);
      analogWrite(MOTOR_RPWM_PIN, 0);
      digitalWrite(MOTOR_RPWM_PIN, LOW);
    }
    delay(4);

    if (engine_ramp_speed < 10)
    {
      engine_ramp_speed = 0;
      engineStop();
      break;
    }
  }
}
void engineStop()
{
  engine_ramp_speed = 0;
  analogWrite(MOTOR_RPWM_PIN, 0);
  analogWrite(MOTOR_LPWM_PIN, 0);
}
