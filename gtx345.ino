#include <Wire.h>
#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include "listener.h"
#include "catalog.h"

#define REG_INPUT_PORT_START 0x00
#define REG_OUTPUT_PORT_START 0x08
#define REG_INT_STATUS_START 0x10
#define REG_PORT_SELECT 0x18
#define REG_INT 0x19
#define REG_PORT_DIRECTION 0x1C
#define REG_PULL_UP 0x1D
#define NUM_PORTS 3

#define ENABLE_3V3_PIN 7

#define OLED_RESET 8
#define OLED_DC 9
#define OLED_CS 10
#define OLED_MOSI 11
#define OLED_CLK 13

Controller &controller(bool reset=false) {
  static U8G2_SSD1322_NHD_256X64_1_4W_HW_SPI screen(U8G2_R2, OLED_CS, OLED_DC, OLED_RESET);
  static Controller controller(&screen);
  return controller;
}

void isr();

class CY8C95x0A {
 public:
  CY8C95x0A(uint8_t i2c_address, int interrrupt_pin) :
    interrrupt_pin_(interrrupt_pin),
    i2c_address_(i2c_address),
    triggered_(false) {}

  void check_error(int error) {
    if (error != 0) {
      Serial.print("I2C Error: ");
      Serial.println(error);
    }
  }
  
  void write_reg(uint8_t reg_offset, uint8_t value) {
    Wire.beginTransmission(i2c_address_);
    Wire.write(reg_offset);
    //check_error(Wire.endTransmission(false));
    Wire.write(value);
    check_error(Wire.endTransmission(true));

    delayMicroseconds(100);
    uint8_t check_value = read_reg(reg_offset);
    while(check_value != value) {
      Serial.print("Register ");
      Serial.print(reg_offset);
      Serial.print(" = ");
      Serial.print(check_value);
      Serial.print(". Wrote ");
      Serial.println(value);
      delay(1000);
      check_value = read_reg(reg_offset);
    }
    delayMicroseconds(100);
  }
  
  void read_regs(uint8_t reg_offset, uint8_t num_bytes, uint8_t * output) {
    Wire.beginTransmission(i2c_address_);
    Wire.write(reg_offset);
    check_error(Wire.endTransmission(false));
    Wire.requestFrom(i2c_address_, num_bytes, (uint8_t)true);
    for (int i = 0; i < num_bytes; ++i) {
      output[i] = Wire.read();
    }
  }

  uint8_t read_reg(uint8_t reg_offset) {
    Wire.beginTransmission(i2c_address_);
    Wire.write(reg_offset);
    check_error(Wire.endTransmission(false));
    Wire.requestFrom(i2c_address_, 1, (uint8_t)true);
    uint8_t val = Wire.read();
    return val;
  }

  void scan() {
    if (!triggered_) return;

    uint8_t int_status[NUM_PORTS];
    uint8_t values[NUM_PORTS];
    read_regs(REG_INT_STATUS_START, NUM_PORTS, int_status);
    read_regs(REG_INPUT_PORT_START, NUM_PORTS, values);
    triggered_ = false;
    int_status[2] &= 0x0F;
    for (int i = 0; i < NUM_PORTS; ++i) {
      int changed = -1;
      switch(int_status[i]) {
        case 0x01:
          changed = 0;
          break;
        case 0x02:
          changed = 1;
          break;
        case 0x04:
          changed = 2;
          break;
        case 0x08:
          changed = 3;
          break;
        case 0x10:
          changed = 4;
          break;
        case 0x20:
          changed = 5;
          break;
        case 0x40:
          changed = 6;
          break;
        case 0x80:
          changed = 7;
          break;
        default:
          continue;
      }
      controller().on_event(i << 4 | changed, (int_status[i] & ~values[i]) != 0);
    }
  }
  
  void begin() {
    Wire.begin();
    attachInterrupt(digitalPinToInterrupt(interrrupt_pin_), isr, RISING);
    for(uint8_t port_num = 0; port_num < NUM_PORTS; ++port_num) {
      write_reg(REG_OUTPUT_PORT_START + port_num, 0);
      write_reg(REG_PORT_SELECT, port_num);
      write_reg(REG_PORT_DIRECTION, 0xff);
      write_reg(REG_PULL_UP, 0xff);
      if (port_num != 2) {
        write_reg(REG_INT, 0x00);
      } else {
        write_reg(REG_INT, 0xF0);
      }
    }
  }

  volatile bool triggered_;
 private:
  int interrrupt_pin_;
  uint8_t i2c_address_;
};


CY8C95x0A expander(0x21, 2);
void isr() {
  expander.triggered_ = true;
}


StatusPage &status_page() {
  static StatusPage internal;
  return internal;
}
SquawkDisplay &squawk_display() {
  static SquawkDisplay internal;
  return internal;
}

StaticText &alt_mode() {
  static StaticText internal("ALT", ALT_X, ALT_Y, ALT_FONT);
  return internal;
}
IdentText &ident_text() {
  static IdentText internal("IDNT", IDNT_X, IDNT_Y, IDNT_FONT);
  return internal;
}
FunctionGroup &xpdr_group() {
  static FunctionGroup internal;
  return internal;
}
FlightId &flight_id() {
  static FlightId internal;
  return internal;
}
FlightIdEdit &flight_id_edit() {
  static FlightIdEdit internal;
  return internal;
}
SerialCom &serial_com() {
  static SerialCom internal;
  return internal;
}

void setup() {
  Serial.begin(115200);
  expander.begin();
  pinMode(ENABLE_3V3_PIN, OUTPUT);
  digitalWrite(ENABLE_3V3_PIN, HIGH);

  controller().add_child(&status_page(), true);
  controller().add_child(&ident_text(), true);
  controller().add_child(&flight_id_edit(), false);
  controller().add_child(&serial_com(), true);
  
  status_page().add_child(&alt_mode(), true);
  status_page().add_child(&squawk_display(), true);
  status_page().add_child(&xpdr_group(), true);

  xpdr_group().add_function(&flight_id());

  controller().begin();
  controller().on_event(EVENT_RENDER);
  Serial.println("Ready");
}

uint32_t last_refresh_ms = 0;
void loop() {
  expander.scan();

  controller().process_event();
  uint32_t now_ms = millis();
  if (now_ms > last_refresh_ms + 100) {
    last_refresh_ms = now_ms;
    controller().on_event(EVENT_TICK);
  }
}
