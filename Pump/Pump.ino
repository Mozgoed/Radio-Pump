/* катушка реле кушает около 60 мА
   Нажатие на ручку энкодера - переключение выбора помпы/периода/времени работы
   Поворот ручки энкодера - изменение значения
   Кнопка энкодера удерживается при включении системы - сброс настроек
   Интерфейс отображается на дисплее 1602 с драйвером на I2C. Версий драйвера существует две.
*/
#define DEBUG 1
class Encoder
{
  public:
    Encoder(uint8_t, uint8_t, uint8_t);

    void setCounters(int norm, int hold);
    void setCounterNorm(int norm);
    void setCounterHold(int hold);

    void setSteps(int norm_step, int hold_step);
    void setStepNorm(int norm_step);
    void setStepHold(int hold_step);

    void setLimitsNorm(int normMin, int normMax);
    void setLimitsHold(int holdMin, int holdMax);

    void invert();
    void tick();
    void setType(boolean type);

    boolean isTurn();

    boolean isRight();
    boolean isLeft();
    boolean isRightH();
    boolean isLeftH();

    boolean isPress();
    boolean isRelease();
    boolean isHolded();
    boolean isHold();

    int normCount, holdCount;

  private:
    byte _CLK, _DT, _SW;

    boolean DT_now, DT_last, SW_state, hold_flag, butt_flag, turn_flag;
    boolean isRight_f, isLeft_f, isRightH_f, isLeftH_f, isTurn_f;
    boolean isPress_f, isRelease_f, isHolded_f, isHold_f;
    boolean _type, _new_step = true;

    int normCountMin, normCountMax, holdCountMin, holdCountMax;
    uint8_t normCount_step, holdCount_step;

    unsigned long debounce_timer;
};

Encoder::Encoder(uint8_t CLK, uint8_t DT, uint8_t SW) {
  _CLK = CLK;
  _DT = DT;
  _SW = SW;
  pinMode (_CLK, INPUT);
  pinMode (_DT, INPUT);
  pinMode (_SW, INPUT_PULLUP);
  DT_last = digitalRead(_CLK);         // читаем начальное положение CLK
}
void Encoder::invert() {
  uint8_t lol = _CLK;
  _CLK = _DT;
  _DT = lol;
}
void Encoder::setType(boolean type) {
  _type = type;
}
void Encoder::setCounters(int norm, int hold) {
  normCount = norm;
  holdCount = hold;
}
void Encoder::setCounterNorm(int norm) {
  normCount = norm;
}
void Encoder::setCounterHold(int hold) {
  holdCount = hold;
}
void Encoder::setSteps(int norm_step, int hold_step) {
  normCount_step = norm_step;
  holdCount_step = hold_step;
}
void Encoder::setStepNorm(int norm_step) {
  normCount_step = norm_step;
}
void Encoder::setStepHold(int hold_step) {
  holdCount_step = hold_step;
}
void Encoder::setLimitsNorm(int normMin, int normMax) {
  normCountMin = normMin;
  normCountMax = normMax;
}
void Encoder::setLimitsHold(int holdMin, int holdMax) {
  holdCountMin = holdMin;
  holdCountMax = holdMax;
}
boolean Encoder::isTurn() {
  if (isTurn_f) {
    isTurn_f = false;
    return true;
  } else return false;
}
boolean Encoder::isRight() {
  if (isRight_f) {
    isRight_f = false;
    return true;
  } else return false;
}
boolean Encoder::isLeft() {
  if (isLeft_f) {
    isLeft_f = false;
    return true;
  } else return false;
}
boolean Encoder::isRightH() {
  if (isRightH_f) {
    isRightH_f = false;
    return true;
  } else return false;
}
boolean Encoder::isLeftH() {
  if (isLeftH_f) {
    isLeftH_f = false;
    return true;
  } else return false;
}
boolean Encoder::isPress() {
  if (isPress_f) {
    isPress_f = false;
    return true;
  } else return false;
}
boolean Encoder::isRelease() {
  if (isRelease_f) {
    isRelease_f = false;
    return true;
  } else return false;
}
boolean Encoder::isHolded() {
  if (isHolded_f) {
    isHolded_f = false;
    return true;
  } else return false;
}
boolean Encoder::isHold() {
  if (isHold_f) {
    isHold_f = false;
    return true;
  } else return false;
}
void Encoder::tick() {
  DT_now = digitalRead(_CLK);          // читаем текущее положение CLK
  SW_state = !digitalRead(_SW);        // читаем положение кнопки SW
  if (SW_state) isHold_f = true;
  else isHold_f = false;

  // отработка нажатия кнопки энкодера
  if (SW_state && !butt_flag && millis() - debounce_timer > 200) {
    hold_flag = false;
    butt_flag = true;
    turn_flag = false;
    debounce_timer = millis();
    isPress_f = true;
  }
  if (!SW_state && butt_flag && millis() - debounce_timer > 200 && millis() - debounce_timer < 500) {
    butt_flag = false;
    if (!turn_flag && !hold_flag) {  // если кнопка отпущена и ручка не поворачивалась
      turn_flag = false;
      isRelease_f = true;
    }
    debounce_timer = millis();
  }

  if (SW_state && butt_flag && millis() - debounce_timer > 800 && !hold_flag) {
    hold_flag = true;
    if (!turn_flag) {  // если кнопка отпущена и ручка не поворачивалась
      turn_flag = false;
      isHolded_f = true;
    }
  }
  if (!SW_state && butt_flag && hold_flag) {
    butt_flag = false;
    debounce_timer = millis();
  }

  if (DT_now != DT_last) {            // если предыдущее и текущее положение CLK разные, значит был поворот
    if (_type) _new_step = !_new_step;
    if (_new_step) {
      if (digitalRead(_DT) != DT_now) {  // если состояние DT отличается от CLK, значит крутим по часовой стрелке
        if (SW_state) {           // если кнопка энкодера нажата
          holdCount += holdCount_step;
          isRightH_f = true;
          isLeftH_f = false;
        } else {                  // если кнопка энкодера не нажата
          normCount += normCount_step;
          isRight_f = true;
          isLeft_f = false;
        }
      } else {                          // если совпадают, значит против часовой
        if (SW_state) {           // если кнопка энкодера нажата
          holdCount -= holdCount_step;
          isLeftH_f = true;
          isRightH_f = false;
        } else {                  // если кнопка энкодера не нажата
          normCount -= normCount_step;
          isLeft_f = true;
          isRight_f = false;
        }
      }
    }
    normCount = constrain(normCount, normCountMin, normCountMax);
    holdCount = constrain(holdCount, holdCountMin, holdCountMax);
    turn_flag = true;                    // флаг что был поворот ручки энкодера
    isTurn_f = true;
  }
  DT_last = DT_now;                   // обновить значение для энкодера
}

//////////////////////////////////////////Настройки энкодера
#define ENCODER_TYPE 0 // Тип энкодера
#define CLK 4
#define DT 3
#define SW 2
Encoder enc(CLK, DT, SW);

//////////////////////////////////////////Настройки дисплея
//#include "LCD_1602_RUS.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#define DRIVER_VERSION 0    // 0 - маркировка драйвера дисплея кончается на 4АТ, 1 - на 4Т
#if (DRIVER_VERSION)
LiquidCrystal_I2C lcd(0x27, 16, 2);
#else
LiquidCrystal_I2C lcd(0x3f, 16, 2);
#endif

//////////////////////////////////////////Настройки реле - мосфетов
#define SWITCH_LEVEL 1 //1 - MOSFET, 0 - Реле
static const wchar_t *relayNames[]  = {
  L"Kitchen",
  L"Hall",
};

#define PARALLEL 0          // 1 - параллельный полив, 0 - полив в порядке очереди
#define TIMER_START 0       // 1 - отсчёт периода с момента ВЫКЛЮЧЕНИЯ помпы, 0 - с момента ВКЛЮЧЕНИЯ помпы
#define PERIOD 1            // 1 - период в часах, 0 - в минутах
#define PUMPING 1           // 1 - время работы помпы в секундах, 0 - в минутах
#define DROP_ICON 1         // 1 - отображать капельку, 0 - будет буква "t" (time)

//#include <EEPROMex.h>
//#include <EEPROMVar.h>

unsigned long pump_timers[2];
unsigned int pumping_time[2];
unsigned int period_time[2];
unsigned int time_left[2];
boolean pump_state[2];
byte pump_pins[2];

byte current_set = 2;
byte current_pump;
boolean reDraw_flag, arrow_update;
boolean now_pumping;
unsigned long period_coef, pumping_coef;

void setup() {
  Serial.begin(9600); //Режим отладки
  //////////////////////////////////////////Выключаем пины
  pinMode(8, OUTPUT); digitalWrite(8, !SWITCH_LEVEL);
  pinMode(9, OUTPUT); digitalWrite(9, !SWITCH_LEVEL);

  //////////////////////////////////////////Включение энкодера
  enc.setStepNorm(1);
  attachInterrupt(0, encISR, CHANGE);
  enc.setType(ENCODER_TYPE);
  enc.setCounters(1, 1);  //Установка начальных переменных
  enc.setSteps(1, 1);     //Установка шага
  enc.setLimitsNorm(0, 1);//Установка пределов
  enc.setLimitsHold(0, 1);//Установка пределов нажатой кнопки

  //////////////////////////////////////////НАСТРОЙКИ
  if (PERIOD) period_coef = (long)1000 * (long)60 * 60;  // перевод в часы
  else period_coef = (long)1000 * 60;              // перевод в минуты

  if (PUMPING) pumping_coef = 1000;          // перевод в секунды
  else pumping_coef = (long)1000 * 60;       // перевод в минуты

  //////////////////////////////////////////Включение дисплея
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // --------------------- СБРОС НАСТРОЕК ---------------------
  //  if (!digitalRead(SW)) {          // если нажат энкодер, сбросить настройки до 1
  //    lcd.setCursor(0, 0);
  //    lcd.print("Reset settings");
  //    for (byte i = 0; i < 100; i++) {
  //      EEPROM.updateByte(i, 1);
  //    }
  //  }
  //  while (!digitalRead(SW));        // ждём отпускания кнопки
  //  lcd.clear();                     // очищаем дисплей, продолжаем работу



  // в ячейке 100 должен быть записан флажок 1, если его нет - делаем (ПЕРВЫЙ ЗАПУСК)
  //  if (EEPROM.read(100) != 1) {
  //    EEPROM.writeByte(100, 1);
  //
  //    // для порядку сделаем 1 ячейки с 0 по 99
  //    for (byte i = 0; i < 100; i++) {
  //      EEPROM.writeByte(i, 1);
  //    }
  //  }
  //
  //  for (byte i = 0; i < PUPM_AMOUNT; i++) {            // пробегаем по всем помпам
  //    period_time[i] = EEPROM.readByte(2 * i);          // читаем данные из памяти. На чётных - период (ч)
  //    pumping_time[i] = EEPROM.readByte(2 * i + 1);     // на нечётных - полив (с)
  //  }

  ///////////////// ---------------------- ВЫВОД НА ДИСПЛЕЙ
  lcd.setCursor(0, 0);
//  lcd.print("Loading"); delay(1000); lcd.print("."); delay(1000); lcd.print("."); delay(1000); lcd.print("."); delay(1000);
  lcd.clear();
  //  lcd.noBacklight();
  //  lcd.noDisplay();
  //  delay(4000);
  //  lcd.display();
  //  delay(1000);
}

byte mode = 0;// 0 - Режим поливайки , 1 - режим радио , 2 - режим ещё чего-то...
bool modePump = false; //Режим поливайки включен или нет
bool modeRadio = false; //Включить или выключить радио
byte page = 0; //Страница в меню
bool sleep = false;
void loop() {
  encoderTick();
  periodTick();
  flowTick();
}

void encoderTick() {
  enc.tick();      // отработка энкодера
  
  if (enc.isTurn())
  {
    if(mode == 0)
    {
      if(page == 0)
      {
        modePump = enc.normCount;
        draw();
      }
    }
    Serial.print(enc.normCount);  // получить счётчик обычный
    Serial.print(" ");
    Serial.println(enc.holdCount); // получить счётчик при нажатой кнопке
  }
  if (enc.isRight()) Serial.println("Right");         // если был поворот
  if (enc.isLeft()) Serial.println("Left");
  if (enc.isRightH()) Serial.println("Right holded"); // если было удержание + поворот
  if (enc.isLeftH()) Serial.println("Left holded");
  if (enc.isPress())
  {
    sleep = false;
    draw();
  }

  if (enc.isRelease())
  {
    Serial.println("Sleep out!");     //Выход из режима сна
    //    lcd.setCursor(1, 0);
    //    lcd.print("Pump #");                       //Обновить дисплей
  }

  if (enc.isHolded()) Serial.println("Holded");       // если была удержана и энк не поворачивался



  //  if (enc.isRelease()) {       // если был нажат
  //    arrow_update = true;        // флаг на обновление стрелочки
  //    reDraw();                   // обновить дисплей
  //  }

  //  if (enc.isTurn()) {                               // если был совершён поворот
  //    switch (current_set) {                           // смотрим, какая опция сейчас меняется
  //      case 0:                                        // если номер помпы
  //        current_pump = enc.normCount;               // получить значение с энкодера
  //        break;
  //      case 1:                                        // если период работы помпы
  //        period_time[current_pump] = enc.normCount;  // получить значение с энкодера
  //        break;
  //      case 2:                                        // если время работы помпы
  //        pumping_time[current_pump] = enc.normCount; // получить значение с энкодера
  //        break;
  //    }
  //    reDraw();                                        // обновить дисплей
  //  }
}


void periodTick() {
  //  for (byte i = 0; i < PUPM_AMOUNT; i++) {            // пробегаем по всем помпам
  //    if ( (millis() - pump_timers[i] > ( (long)period_time[i] * period_coef) )
  //         && (pump_state[i] != SWITCH_LEVEL)
  //         && !(now_pumping * !PARALLEL)) {
  //      pump_state[i] = SWITCH_LEVEL;
  //      digitalWrite(pump_pins[i], SWITCH_LEVEL);
  //      pump_timers[i] = millis();
  //      now_pumping = true;
  //    }
  //  }
}

void flowTick() {
  //  for (byte i = 0; i < PUPM_AMOUNT; i++) {            // пробегаем по всем помпам
  //    if ( (millis() - pump_timers[i] > ( (long)pumping_time[i] * pumping_coef) )
  //         && (pump_state[i] == SWITCH_LEVEL) ) {
  //      pump_state[i] = !SWITCH_LEVEL;
  //      digitalWrite(pump_pins[i], !SWITCH_LEVEL);
  //      if (TIMER_START) pump_timers[i] = millis();
  //      now_pumping = false;
  //    }
  //  }
}

void encISR() {
  enc.tick();                  // отработка энкодера
}

void draw() {
  lcd.clear();
  
  if (mode == 0)
  {
    if (page == 0)
    {
      lcd.home();
      lcd.print("* Mode : Pump *");
      lcd.setCursor(0,1);
      lcd.print("> Work : ");
      if(modePump) lcd.print("ON"); else lcd.print("OFF");
    }
    if (page == 1)
    {
      lcd.home();
      lcd.print("* Mode : Pump *");
      lcd.setCursor(0,1);
      lcd.print("> Work : ");
      if(modePump) lcd.print("ON"); else lcd.print("OFF");
    }
  }
  //  if (arrow_update) {                            // если изменился режим выбора
  //    if (++current_set > 2)                       // менять current_set в пределах 0.. 2
  //      current_set = 0;
  //    if (current_set == 0) update_EEPROM();       // если переключилиссь на выбор помпы, обновить данные
  //    switch (current_set) {                       // смотрим, какая опция сейчас выбрана
  //      case 0:                                    // если номер помпы
  //        enc.setCounterNorm(current_pump);       // говорим энкодеру работать с номером помпы
  //        enc.setLimitsNorm(0, PUPM_AMOUNT - 1);  // ограничиваем
  //        // стереть предыдущую стрелочку и нарисовать новую
  //        lcd.setCursor(0, 0); lcd.write(126); lcd.setCursor(9, 1); lcd.print(" ");
  //        break;
  //      case 1:
  //        enc.setCounterNorm(period_time[current_pump]);
  //        enc.setLimitsNorm(1, 99);
  //        lcd.setCursor(0, 1); lcd.write(126); lcd.setCursor(0, 0); lcd.print(" ");
  //        break;
  //      case 2:
  //        enc.setCounterNorm(pumping_time[current_pump]);
  //        enc.setLimitsNorm(1, 99);
  //        lcd.setCursor(9, 1); lcd.write(126); lcd.setCursor(0, 1); lcd.print(" ");
  //        break;
  //    }
  //    arrow_update = false;
  //  } else {
  //    // вывести все цифровые значения на их места
  //    //lcd.setCursor(7, 0);
  //    //lcd.print(current_pump);
  //    if (current_set == 0) {
  //      lcd.setCursor(1, 0);
  //      lcd.print("              ");
  //      lcd.setCursor(1, 0);
  //      lcd.print(relayNames[current_pump]);
  //    }
  //
  //    lcd.setCursor(5, 1);
  //    lcd.print(period_time[current_pump]);
  //    if (PERIOD) lcd.print("h ");
  //    else lcd.print("m ");
  //    lcd.setCursor(12, 1);
  //    lcd.print(pumping_time[current_pump]);
  //    if (PUMPING) lcd.print("s ");
  //    else lcd.print("m ");
  //    /*
  //        Serial.print("Pump #"); Serial.print(current_pump);
  //            if (PERIOD) Serial.print(" hrs, period: ");
  //            else Serial.print(" min, period: ");
  //            Serial.print(period_time[current_pump]);
  //            if (PUMPING) Serial.print(" sec, flow: ");
  //            else Serial.print(" min, flow: ");
  //            Serial.println(pumping_time[current_pump]);
  //    */
  //  }
}

// обновляем данные в памяти
void update_EEPROM() {
  //  EEPROM.updateByte(2 * current_pump, period_time[current_pump]);
  //  EEPROM.updateByte(2 * current_pump + 1, pumping_time[current_pump]);
}

