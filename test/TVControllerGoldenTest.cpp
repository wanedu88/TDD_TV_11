#include "fakeTuner.h"
#include "golden/ControllerTrace.h"
#include "golden/GoldenMaster.h"
#include <functional>
#include <gtest/gtest.h>
#include <memory>

namespace {

void runScenario(const char *name,
                 std::function<void(ControllerTrace &)> body) {
  SCOPED_TRACE(name);
  FakeTuner tuner(std::vector<int>{1, 4, 12, 56});
  TVChannelController ctrl(tuner);
  ControllerTrace trace(ctrl, tuner);
  trace.header(std::string("fixture=FakeTuner{1,4,12,56} scenario=") + name);
  body(trace);
  const golden::GoldenResult result = golden::assertGolden(trace.str(), name);
  if (!result.match) {
    FAIL() << result.message;
  }
}

void runSearchListScenario(const char *name,
                           std::function<void(ControllerTrace &)> body) {
  SCOPED_TRACE(name);
  FakeTuner tuner(std::vector<int>{4, 6, 14});
  TVChannelController ctrl(tuner);
  ControllerTrace trace(ctrl, tuner);
  trace.header(std::string("fixture=FakeTuner{4,6,14} scenario=") + name);
  body(trace);
  const golden::GoldenResult result = golden::assertGolden(trace.str(), name);
  if (!result.match) {
    FAIL() << result.message;
  }
}

} // namespace

// ─── FR-01: 숫자 버튼 채널 변경 ───────────────────────────────

TEST(TVControllerGolden, fr01_press1_confirm) {
  runScenario("fr01_press1_confirm", [](ControllerTrace &t) {
    t.pressNumber(1);
    t.pressConfirm();
  });
}

TEST(TVControllerGolden, fr01_press12_auto) {
  runScenario("fr01_press12_auto", [](ControllerTrace &t) {
    t.pressNumber(1);
    t.pressNumber(2);
  });
}

TEST(TVControllerGolden, fr01_press1234) {
  runScenario("fr01_press1234", [](ControllerTrace &t) {
    t.pressNumber(1);
    t.pressNumber(2);
    t.pressNumber(3);
    t.pressNumber(4);
  });
}

TEST(TVControllerGolden, fr01_three_digits_456_confirm) {
  runScenario("fr01_three_digits_456_confirm", [](ControllerTrace &t) {
    t.pressNumber(4);
    t.pressNumber(5);
    t.pressNumber(6);
    t.pressConfirm();
  });
}

TEST(TVControllerGolden, fr01_three_digits_456_other) {
  runScenario("fr01_three_digits_456_other", [](ControllerTrace &t) {
    t.pressNumber(4);
    t.pressNumber(5);
    t.pressNumber(6);
    t.pressOther();
  });
}

TEST(TVControllerGolden, fr01_zero7) {
  runScenario("fr01_zero7", [](ControllerTrace &t) {
    t.pressNumber(0);
    t.pressNumber(7);
  });
}

// ─── FR-02 / FR-03: 선호 채널 ─────────────────────────────────

TEST(TVControllerGolden, fr02_favorite_toggle_scenario) {
  runScenario("fr02_favorite_toggle_scenario", [](ControllerTrace &t) {
    for (int ch : {12, 8, 37, 8, 6}) {
      t.setChannel(ch);
      t.pressFavorite();
    }
  });
}

TEST(TVControllerGolden, fr03_next_favorite_from6) {
  runScenario("fr03_next_favorite_from6", [](ControllerTrace &t) {
    for (int ch : {1, 4, 12, 56}) {
      t.addFavorite(ch);
    }
    t.setChannel(6);
    t.pressNextFavorite();
  });
}

TEST(TVControllerGolden, fr03_next_favorite_wrap56) {
  runScenario("fr03_next_favorite_wrap56", [](ControllerTrace &t) {
    t.addFavorite(1);
    t.addFavorite(56);
    t.setChannel(56);
    t.pressNextFavorite();
  });
}

// ─── FR-04: 채널 검색 ─────────────────────────────────────────

TEST(TVControllerGolden, fr04_channel_search) {
  runScenario("fr04_channel_search", [](ControllerTrace &t) {
    t.pressChannelSearch();
  });
}

TEST(TVControllerGolden, fr04_search_then_up_from6) {
  runScenario("fr04_search_then_up_from6", [](ControllerTrace &t) {
    t.pressChannelSearch();
    t.setChannel(6);
    t.pressChannelUp();
  });
}

// ─── FR-05: 업/다운 (검색 없음) ───────────────────────────────

TEST(TVControllerGolden, fr05_up_down_no_search) {
  runScenario("fr05_up_down_no_search", [](ControllerTrace &t) {
    t.setChannel(6);
    t.pressChannelUp();
    t.pressChannelDown();
    t.pressChannelDown();
    t.setChannel(99);
    t.pressChannelUp();
    t.setChannel(0);
    t.pressChannelDown();
  });
}

// ─── FR-06: 업/다운 (검색 목록) ───────────────────────────────

TEST(TVControllerGolden, fr06_up_down_with_search) {
  runSearchListScenario("fr06_up_down_with_search", [](ControllerTrace &t) {
    t.pressChannelSearch();
    t.setChannel(6);
    t.pressChannelUp();
    t.pressChannelDown();
    t.setChannel(15);
    t.pressChannelUp();
    t.setChannel(15);
    t.pressChannelDown();
    t.setChannel(4);
    t.pressChannelDown();
  });
}

TEST(TVControllerGolden, fr06_up_down_cycle) {
  runSearchListScenario("fr06_up_down_cycle", [](ControllerTrace &t) {
    t.pressChannelSearch();
    t.setChannel(6);
    t.pressChannelUp();
    t.pressChannelDown();
  });
}
