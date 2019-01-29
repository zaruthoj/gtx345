#include <cstddef>
#include "../catalog.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "u8g2_mock.h"

const uint8_t * u8g2_font_inb30_mr=nullptr;
const uint8_t * u8g2_font_inb16_mr=nullptr;
const uint8_t * u8g2_font_7x13B_tf=nullptr;
const uint8_t * u8g2_font_7x13_mf=nullptr;
const uint8_t * u8g2_font_9x18B_mf=nullptr;
uint32_t mock_time_ms=1000;


void reset_mock_time() {
  mock_time_ms = 1000;
}

void advance_mock_time(uint32_t ms) {
  mock_time_ms += ms;
}

uint32_t millis() {
  return mock_time_ms;
}

void SerialClass::print(const char* data) {}
void SerialClass::println(const char* data) {}

SerialClass Serial;

using testing::NiceMock;
using testing::_;
using testing::StrEq;
using testing::Mock;
using testing::AtLeast;
using testing::InSequence;
using testing::Return;

class U8G2Mock : public U8G2 {
 public:
  MOCK_METHOD1(setDrawColor, void(uint8_t));
  MOCK_METHOD1(setFont, void(const uint8_t *));
  MOCK_METHOD3(drawStr, void(uint8_t, uint8_t, const char *));
  MOCK_METHOD3(drawGlyph, void(uint8_t, uint8_t, int16_t));
};

NiceMock<U8G2Mock> &screen() {
  static NiceMock<U8G2Mock> screen;
  return screen;
}

Controller &controller(bool reset=false) {
  static Controller *controller(new Controller(&screen()));
  if (reset) {
    delete controller;
    controller = new Controller(&screen());
  }
  return *controller;
}

StaticText &alt_mode() {
  static StaticText alt_mode("ALT", 0, 45, u8g2_font_7x13B_tf);
  return alt_mode;
}
StatusPage &status_page() {
  static StatusPage internal;
  return internal;
}
FlightIdEdit &flight_id_edit() {
  static FlightIdEdit internal;
  return internal;
}

class EditTest : public ::testing::Test {
 protected:
  EditTest() :
      squawk_(new SquawkDisplay()),
      flight_id_(new FlightIdEdit()) {
    Mock::VerifyAndClear(&screen());
    controller(true);
    reset_mock_time();
    controller().add_child(squawk_, false);
    controller().add_child(flight_id_, false);
  }

  ~EditTest() override {
    delete squawk_;
    delete flight_id_;
  }

  SquawkDisplay *squawk_;
  FlightIdEdit *flight_id_;
};

TEST_F(EditTest, TestNonEditRender) {
  InSequence dummy;
  
  controller().activate_child(squawk_);
  EXPECT_CALL(screen(), drawStr(_, _, StrEq("1200"))).Times(1);
  squawk_->on_event(EVENT_RENDER);
}

TEST_F(EditTest, TestSquawkDigitEntry) {
  InSequence dummy;
  
  controller().activate_child(squawk_);
  EXPECT_CALL(screen(), drawStr(_, _, StrEq("0"))).Times(1);
  EXPECT_CALL(screen(), drawGlyph(_, _, '_')).Times(3);
  squawk_->on_event(BUTTON_D0, true);
  controller().process_event();
  
  EXPECT_CALL(screen(), drawStr(_, _, StrEq("00"))).Times(1);
  EXPECT_CALL(screen(), drawGlyph(_, _, '_')).Times(2);
  squawk_->on_event(BUTTON_D0, true);
  controller().process_event();

  EXPECT_CALL(screen(), drawStr(_, _, StrEq("001"))).Times(1);
  EXPECT_CALL(screen(), drawGlyph(_, _, '_')).Times(1);
  squawk_->on_event(BUTTON_D1, true);
  controller().process_event();
  
  squawk_->on_event(BUTTON_D8, true);
  controller().process_event();

  EXPECT_CALL(screen(), drawStr(_, _, StrEq("0012"))).Times(1);
  squawk_->on_event(BUTTON_D2, true);
  controller().process_event();

  EXPECT_STREQ(controller().squawk_code_, "0012");
  EXPECT_STREQ(controller().old_code_, "");
}

TEST_F(EditTest, TestSquawkDigitClear) {
  InSequence dummy;
  
  controller().activate_child(squawk_);
  squawk_->on_event(BUTTON_D0, true);
  controller().process_event();  
  squawk_->on_event(BUTTON_D1, true);
  controller().process_event();
  EXPECT_STREQ(controller().squawk_code_, "1200");

  squawk_->on_event(BUTTON_CLR, true);
  squawk_->on_event(BUTTON_CLR, false);
  EXPECT_CALL(screen(), drawStr(_, _, StrEq("0"))).Times(1);
  EXPECT_CALL(screen(), drawGlyph(_, _, '_')).Times(3);
  controller().process_event();
  
  squawk_->on_event(BUTTON_CLR, true);
  squawk_->on_event(BUTTON_CLR, false);
  EXPECT_CALL(screen(), drawStr(_, _, StrEq(""))).Times(1);
  EXPECT_CALL(screen(), drawGlyph(_, _, '_')).Times(4);
  controller().process_event();

  squawk_->on_event(BUTTON_CLR, true);
  squawk_->on_event(BUTTON_CLR, false);
  EXPECT_CALL(screen(), drawStr(_, _, StrEq("1200"))).Times(1);
  controller().process_event();

  EXPECT_STREQ(controller().squawk_code_, "1200");
}

TEST_F(EditTest, TestSquawkDigitClearAll) {
  {
    InSequence dummy;
    
    controller().activate_child(squawk_);
    squawk_->on_event(BUTTON_D0, true);
    controller().process_event();  
    squawk_->on_event(BUTTON_D1, true);
    controller().process_event();
    squawk_->on_event(BUTTON_D2, true);
    controller().process_event();
    EXPECT_STREQ(controller().squawk_code_, "1200");

    squawk_->on_event(BUTTON_CLR, true);
    squawk_->on_event(EVENT_TICK);
    advance_mock_time(1000);
    squawk_->on_event(EVENT_TICK);
    squawk_->on_event(BUTTON_CLR, false);
    EXPECT_CALL(screen(), drawStr(_, _, StrEq(""))).Times(1);
    EXPECT_CALL(screen(), drawGlyph(_, _, '_')).Times(4);
    controller().process_event();
    EXPECT_STREQ(controller().squawk_code_, "1200");
  }
  
  Mock::VerifyAndClear(&screen());
 
  squawk_->on_event(BUTTON_D3, true);
  controller().process_event();  
  squawk_->on_event(BUTTON_D2, true);
  controller().process_event();
  squawk_->on_event(BUTTON_D1, true);
  controller().process_event();
  squawk_->on_event(BUTTON_D0, true);
  controller().process_event();

  EXPECT_STREQ(controller().squawk_code_, "3210");
  EXPECT_STREQ(controller().old_code_, "");

}

TEST_F(EditTest, TestSquawkDigitCursor) {
  InSequence dummy;
  
  controller().activate_child(squawk_);
  squawk_->on_event(BUTTON_D0, true);
  controller().process_event();  
  squawk_->on_event(BUTTON_D1, true);
  controller().process_event();
  EXPECT_STREQ(controller().squawk_code_, "1200");

  squawk_->on_event(BUTTON_CRSR, true);
  EXPECT_CALL(screen(), drawStr(_, _, StrEq("1200"))).Times(1);
  controller().process_event();

  EXPECT_STREQ(controller().squawk_code_, "1200");
}

TEST_F(EditTest, TestSquawkDigitVfr) {
  controller().activate_child(squawk_);
  squawk_->on_event(BUTTON_D0, true);
  controller().process_event();  
  squawk_->on_event(BUTTON_D1, true);
  controller().process_event();
  squawk_->on_event(BUTTON_D2, true);
  controller().process_event();  
  squawk_->on_event(BUTTON_D3, true);
  controller().process_event();
  EXPECT_STREQ(controller().squawk_code_, "0123");

  squawk_->on_event(BUTTON_VFR, true);
  controller().process_event();
  EXPECT_STREQ(controller().squawk_code_, "1200");

  squawk_->on_event(BUTTON_VFR, true);
  controller().process_event();
  EXPECT_STREQ(controller().squawk_code_, "0123");
}

TEST_F(EditTest, TestFlightIdNumberEntry) {
  controller().activate_child(flight_id_);
  
  flight_id_->on_event(BUTTON_D0, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '0');

  flight_id_->on_event(BUTTON_D1, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "0");
  EXPECT_EQ(flight_id_->get_cursor_char(), '1');

  flight_id_->on_event(BUTTON_D2, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "01");
  EXPECT_EQ(flight_id_->get_cursor_char(), '2');
 
  flight_id_->on_event(BUTTON_D3, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "012");
  EXPECT_EQ(flight_id_->get_cursor_char(), '3');
  
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "0123");
  EXPECT_EQ(flight_id_->get_cursor_char(), '4');
  
  flight_id_->on_event(BUTTON_ENT, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');

  EXPECT_STREQ(controller().flight_id_, "01234");
}

TEST_F(EditTest, TestFlightIdLetterNumberEntry) {
  controller().activate_child(flight_id_);
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '4');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), 'M');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), 'N');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D2, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "N");
  EXPECT_EQ(flight_id_->get_cursor_char(), '2');
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_ENT, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  
  EXPECT_STREQ(controller().flight_id_, "N2");
}

TEST_F(EditTest, TestFlightIdLetterWaitEntry) {
  controller().activate_child(flight_id_);
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D8, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '8');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D8, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), 'Y');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D8, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), 'Z');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D8, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '8');
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D8, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), 'Y');
  
  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D8, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "Y");
  EXPECT_EQ(flight_id_->get_cursor_char(), '8');
  
  flight_id_->on_event(BUTTON_ENT, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');

  EXPECT_STREQ(controller().flight_id_, "Y8");
}

TEST_F(EditTest, TestFlightId9DigitEntry) {
  controller().activate_child(flight_id_);
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D9, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "9");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D9, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "99");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);

  flight_id_->on_event(BUTTON_ENT, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');
  EXPECT_STREQ(controller().flight_id_, "99");
}

TEST_F(EditTest, TestFlightIdMixEntry) {
  controller().activate_child(flight_id_);
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D9, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D2, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D1, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D1, true);
  controller().process_event();

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D1, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D1, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D1, true);
  controller().process_event();

  flight_id_->on_event(BUTTON_ENT, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');
  EXPECT_STREQ(controller().flight_id_, "N924DE");
}

TEST_F(EditTest, TestFlightIdFullLengthEntry) {
  controller().activate_child(flight_id_);
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D9, true);
  controller().process_event();

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D2, true);
  controller().process_event();

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D4, true);
  controller().process_event();

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D0, true);
  controller().process_event();

 advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D8, true);
  controller().process_event();

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D1, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D1, true);
  controller().process_event();

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D6, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D6, true);
  controller().process_event();

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D6, true);
  controller().process_event();
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_ENT);
  controller().process_event();
  
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');
  EXPECT_STREQ(controller().flight_id_, "N92408DT");
}

TEST_F(EditTest, TestFlightIdPartialLetterClearEntry) {
  controller().activate_child(flight_id_);

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D6, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '6'); 
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D7, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "6");
  EXPECT_EQ(flight_id_->get_cursor_char(), '7');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D7, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "6");
  EXPECT_EQ(flight_id_->get_cursor_char(), 'V');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_CLR, true);
  flight_id_->on_event(BUTTON_CLR, false);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "6");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D5, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "6");
  EXPECT_EQ(flight_id_->get_cursor_char(), '5');
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_ENT, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');
  
  EXPECT_STREQ(controller().flight_id_, "65");
}

TEST_F(EditTest, TestFlightIdCompleteLetterClearEntry) {
  controller().activate_child(flight_id_);

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D6, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '6'); 
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D7, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "6");
  EXPECT_EQ(flight_id_->get_cursor_char(), '7');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D7, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "6");
  EXPECT_EQ(flight_id_->get_cursor_char(), 'V');

  advance_mock_time(3000);
  flight_id_->on_event(EVENT_TICK);
  EXPECT_STREQ(flight_id_->edit_text_, "6V");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');

  flight_id_->on_event(BUTTON_CLR, true);
  flight_id_->on_event(BUTTON_CLR, false);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "6");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');

  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_D5, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "6");
  EXPECT_EQ(flight_id_->get_cursor_char(), '5');
  
  advance_mock_time(200);
  flight_id_->on_event(EVENT_TICK);
  flight_id_->on_event(BUTTON_ENT, true);
  controller().process_event();
  EXPECT_STREQ(flight_id_->edit_text_, "");
  EXPECT_EQ(flight_id_->get_cursor_char(), '_');
  
  EXPECT_STREQ(controller().flight_id_, "65");
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
      
