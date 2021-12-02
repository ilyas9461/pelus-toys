/* DF player seslere ait tanımlar*/

#define MP3_ACC_VOLTAGE 8
#define MP3_GAME_OVER 9
#define MP3_CAR_START 10
#define MP3_ACCIDENT 11
#define MP3_HEY 12
#define MP3_HORN_OLDEST_CAR 13
#define MP3_HORN_TRUCK 14
#define MP3_APPLAUSE 15
#define MP3_WARNING 16
#define MP3_BABY_SHARK 17
#define MP3_HOCKEY 18
#define MP3_BEGIN 19

//// bosta rastgele mp3 müzik

#define mp3_adele_Rolling 20
#define mp3_anitta 21
#define mp3_love_lo 22
#define mp3_remix 23
#define mp3_dejavu 24
#define mp3_rain 25
#define _mp3_aweli 26

/* PIN Tanımlamları*/
#define DF_BUSSY_PIN 7
#define DF_PLAYER_ARDU_RX_PIN 8 // DF nin TX pininden veri alır  ARDU_RX_D8 <-- DF TX PIN
#define DF_PLAYER_ARDU_TX_PIN 9 // DF TX pinine veri gönderir    ARDU_TX_D9 --> DF RX PIN

#define BUZZER_PIN 10

#define JTN_PIN A3

#define BTN_LEFT_PIN 4
#define BTN_OK_PIN 5
#define BTN_RIGHT_PIN 6

#define DIR_SW_PIN 12
#define GAS_PEDAL_PIN 13  // If PIN13 is digital input use, 
                          // the board LED is must be disassmbly.
#define MZ80_PIN 2

#define VAKU_PIN A6
#define IAKU_PIN A7

#define DISP_BEKLE_SURE 2000
#define BTN_DELAY 300

#define ACC_V_READ_DELAY -3  //Delay time for accumulator voltage read 

#define ARDU_VOLTAGE 5.0 // Operating voltage of Arduino. either 3.3V or 5.0V
#define MAX_AKIM 3.0

#define EE_ADR_GAME_TIME 0        // Oyun süresi için EEPROM adresi dakika olarak
#define EE_ADR_SPEED 10            // HIZ ayarı 0-255 arası
#define EE_ADR_VOLUME 20           // MP3 ses 0-30 arası
#define EE_ADR_STOP_TIME 30        // Soft stop zamanı saniye olarak
#define EE_ADR_SPEED_TIME 40      // Soft speed zamanı saniye olarak.
#define EE_ADR_FRONT_SENS_STAT 50 // ön sensör durum

#define EE_ADR_MAX_I_INT_VALUE 60
#define EE_ADR_MAX_I_FLOAT_VALUE 70

#define EE_ADR_COUNT_TOKEN 80

/* BTS 7960 */

#define MOTOR_RPWM_PIN 11
#define MOTOR_LPWM_PIN 3

// #define motorIleriGit(hiz)  analogWrite(MOTOR_LPWM_PIN, 0);digitalWrite(MOTOR_LPWM_PIN, LOW);\
//                             analogWrite(MOTOR_RPWM_PIN, hiz)
// #define motorGeriGit(hiz)   analogWrite(MOTOR_LPWM_PIN, hiz);\
//                             analogWrite(MOTOR_RPWM_PIN, 0);digitalWrite(MOTOR_RPWM_PIN, LOW)

// #define motorDur() analogWrite(MOTOR_RPWM_PIN, 0);\
//                    analogWrite(MOTOR_LPWM_PIN, 0);

// #define SISLEME_OUT_PIN 13
// #define sislemeOn() digitalWrite(SISLEME_OUT_PIN, LOW);
// #define sislemeOff() digitalWrite(SISLEME_OUT_PIN, HIGH);

/* RGB ledlerle ilgili tanımlamlar */

#define R_LED_PIN A2
#define G_LED_PIN A1
#define B_LED_PIN A0

#define rgb_initial()           \
    pinMode(R_LED_PIN, OUTPUT); \
    pinMode(G_LED_PIN, OUTPUT); \
    pinMode(B_LED_PIN, OUTPUT)

#define r_led_on()                 \
    digitalWrite(R_LED_PIN, HIGH); \
    digitalWrite(G_LED_PIN, LOW);  \
    digitalWrite(B_LED_PIN, LOW)

#define g_led_on()                 \
    digitalWrite(R_LED_PIN, LOW);  \
    digitalWrite(G_LED_PIN, HIGH); \
    digitalWrite(B_LED_PIN, LOW)

#define b_led_on()                \
    digitalWrite(R_LED_PIN, LOW); \
    digitalWrite(G_LED_PIN, LOW); \
    digitalWrite(B_LED_PIN, HIGH)

#define rgb_off()                 \
    digitalWrite(R_LED_PIN, LOW); \
    digitalWrite(G_LED_PIN, LOW); \
    digitalWrite(B_LED_PIN, LOW)
