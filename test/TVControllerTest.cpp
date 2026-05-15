#include "TVChannelController.h"
#include "fakeTuner.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockTunerForController : public Tuner {
public:
  MOCK_METHOD(std::string, seekCH, (), (override));
  MOCK_METHOD(void, setCH, (const std::string &ch), (override));
  MOCK_METHOD(std::string, getCurrentCH, (), (override));
};

class ControllerTest : public ::testing::Test {
protected:
  std::unique_ptr<FakeTuner> tuner;
  std::unique_ptr<TVChannelController> ctrl;

  void SetUp() override {
    tuner = std::make_unique<FakeTuner>(std::vector<int>{1, 4, 12, 56});

    ctrl = std::make_unique<TVChannelController>(*tuner);
  }
};

// ─── 기능1: 숫자버튼채널변경─────────────────────────────
// S1-1: 한자리입력+ 확인
TEST_F(ControllerTest, PressNumber1ThenConfirm) {
  ctrl->pressNumber(1);                  // Given
  ctrl->pressConfirm();                  // When
  EXPECT_EQ("1", tuner->getCurrentCH()); // Then
}
// // S1-2: 두자리자동변경
TEST_F(ControllerTest, Press1Then2_AutoChange) {
  ctrl->pressNumber(1);
  ctrl->pressNumber(2); // 두자리완성→ 자동
  EXPECT_EQ("12", tuner->getCurrentCH());
}
// // S1-3: 연속두자리×2회
TEST_F(ControllerTest, Press1234_TwoStageChange) {
  ctrl->pressNumber(1);
  ctrl->pressNumber(2); // → 12
  ctrl->pressNumber(3);
  ctrl->pressNumber(4); // → 34
  EXPECT_EQ("34", tuner->getCurrentCH());
}
// // S1-4: 버퍼무효화
TEST_F(ControllerTest, OtherButtonCancelsBuffer) {
  ctrl->pressNumber(4);
  ctrl->pressNumber(5);
  ctrl->pressNumber(6); // 3자리→ 무효화
  ctrl->pressOther();
  // 6은무효화→ 채널변화없음
  EXPECT_EQ("0", tuner->getCurrentCH());
}
// // S1-5: '0','7' → 7번
TEST_F(ControllerTest, Zero7_SingleDigit7) {
  ctrl->pressNumber(0);
  ctrl->pressNumber(7);
  EXPECT_EQ("7", tuner->getCurrentCH());
}

// // ─── 기능 2: 선호 채널 토글 ──────────────────────────────────
TEST_F(ControllerTest, FavoriteAdd_NewChannel) {
  tuner->setCH("12");
  ctrl->pressFavorite();
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
}
TEST_F(ControllerTest, FavoriteToggle_Remove) {
  tuner->setCH("12");
  ctrl->pressFavorite(); // 추가
  ctrl->pressFavorite(); // 삭제 (토글)
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_EQ(favs.end(), std::find(favs.begin(), favs.end(), 12));
}
// S2-3: 토글 시나리오 전체
TEST_F(ControllerTest, FavoriteToggleScenario) {
  for (int ch : {12, 8, 37, 8, 6}) {
    tuner->setCH(std::to_string(ch));
    ctrl->pressFavorite();
  }
  const auto &favs = ctrl->getFavoriteChannels();
  // {6, 12, 37} 만 남아야 함
  EXPECT_EQ(3u, favs.size());
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 6));
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 37));
}

// // ─── 기능 3: 다음 선호 채널
TEST_F(ControllerTest, NextFavorite_Normal) {
  for (int ch : {1, 4, 12, 56})
    ctrl->addFavorite(ch);
  tuner->setCH("6");
  ctrl->pressNextFavorite();
  EXPECT_EQ("12", tuner->getCurrentCH());
}
TEST_F(ControllerTest, NextFavorite_WrapAround) {
  ctrl->addFavorite(1);
  ctrl->addFavorite(56);
  tuner->setCH("56");
  ctrl->pressNextFavorite();
  EXPECT_EQ("1", tuner->getCurrentCH());
}
TEST_F(ControllerTest, NextFavorite_EmptyList) {
  tuner->setCH("6");
  ctrl->pressNextFavorite();
  EXPECT_EQ("6", tuner->getCurrentCH()); // 변화 없음
}
