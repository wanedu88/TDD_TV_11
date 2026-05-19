#include "TVChannelController.h"
#include "fakeTuner.h"
#include <algorithm>
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

  int currentCh() const { return std::stoi(tuner->getCurrentCH()); }
};

class ChannelSearchMockTest : public ::testing::Test {
protected:
  MockTunerForController mockTuner;
  std::unique_ptr<TVChannelController> ctrl;

  void SetUp() override {
    ctrl = std::make_unique<TVChannelController>(mockTuner);
  }
};

class ControllerSearchListTest : public ::testing::Test {
protected:
  std::unique_ptr<FakeTuner> tuner;
  std::unique_ptr<TVChannelController> ctrl;

  void SetUp() override {
    tuner = std::make_unique<FakeTuner>(std::vector<int>{4, 6, 14});
    ctrl = std::make_unique<TVChannelController>(*tuner);
  }

  int currentCh() const { return std::stoi(tuner->getCurrentCH()); }

  void searchAndSetChannel(int ch) {
    ctrl->pressChannelSearch();
    tuner->setCH(std::to_string(ch));
  }
};

// ─── FR-01: 숫자 버튼 채널 변경 ───────────────────────────────

TEST_F(ControllerTest, PressNumber1ThenConfirm) {
  // Given: 초기 채널 0
  // When: '1' 입력 후 확인
  ctrl->pressNumber(1);
  ctrl->pressConfirm();
  // Then: 1번 채널
  EXPECT_EQ(1, currentCh());
}

TEST_F(ControllerTest, Press1Then2_AutoChange) {
  // Given: 초기 채널 0
  // When: '1' 후 '2' (두 자리 자동 적용)
  ctrl->pressNumber(1);
  ctrl->pressNumber(2);
  // Then: 12번 채널
  EXPECT_EQ(12, currentCh());
}

TEST_F(ControllerTest, Press1234_TwoStageChange) {
  // Given: 초기 채널 0
  // When: '1','2','3','4' 연속 입력
  ctrl->pressNumber(1);
  ctrl->pressNumber(2);
  ASSERT_EQ(12, currentCh());
  ctrl->pressNumber(3);
  ctrl->pressNumber(4);
  // Then: 최종 34번 채널
  EXPECT_EQ(34, currentCh());
}

TEST_F(ControllerTest, ThreeDigits_ApplyFirstTwoThenConfirm) {
  // Given: 초기 채널 0
  // When: '4','5','6' 후 확인 (세 자리 경계)
  ctrl->pressNumber(4);
  ctrl->pressNumber(5);
  ASSERT_EQ(45, currentCh());
  ctrl->pressNumber(6);
  ctrl->pressConfirm();
  // Then: 버퍼의 6 한 자리 적용
  EXPECT_EQ(6, currentCh());
}

TEST_F(ControllerTest, ThreeDigits_ThenOther_ClearsSix) {
  // Given: '4','5','6' 입력 후 45번 채널
  ctrl->pressNumber(4);
  ctrl->pressNumber(5);
  ctrl->pressNumber(6);
  ASSERT_EQ(45, currentCh());
  // When: 확인·숫자 외 버튼
  ctrl->pressOther();
  // Then: 6 무효화, 45 유지
  EXPECT_EQ(45, currentCh());
}

TEST_F(ControllerTest, Zero7_SingleDigit7) {
  // Given: 초기 채널 0
  // When: '0' 후 '7'
  ctrl->pressNumber(0);
  ctrl->pressNumber(7);
  // Then: 7번 채널
  EXPECT_EQ(7, currentCh());
}

TEST_F(ControllerTest, SingleDigit0_ThenConfirm) {
  // Given: 초기 채널 0
  // When: '0' 후 확인
  ctrl->pressNumber(0);
  ctrl->pressConfirm();
  // Then: 0번 채널
  EXPECT_EQ(0, currentCh());
}

TEST_F(ControllerTest, MaxChannel99_AutoTwoDigits) {
  // Given: 초기 채널 0
  // When: '9' 후 '9'
  ctrl->pressNumber(9);
  ctrl->pressNumber(9);
  // Then: 99번 채널
  EXPECT_EQ(99, currentCh());
}

// ─── FR-02: 선호 채널 추가/삭제 ───────────────────────────────

TEST_F(ControllerTest, FavoriteAdd_NewChannel) {
  // Given: 12번 채널 시청 중, 선호 목록에 없음
  tuner->setCH("12");
  // When: 선호채널추가
  ctrl->pressFavorite();
  // Then: 12가 선호 목록에 포함
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

TEST_F(ControllerTest, FavoriteToggle_Remove) {
  // Given: 12번이 선호 채널로 등록됨
  tuner->setCH("12");
  ctrl->pressFavorite();
  // When: 선호채널추가 재입력 (토글)
  ctrl->pressFavorite();
  // Then: 목록에서 제거
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_EQ(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

TEST_F(ControllerTest, FavoriteToggleScenario) {
  // Given: 여러 채널에서 선호 토글
  for (int ch : {12, 8, 37, 8, 6}) {
    tuner->setCH(std::to_string(ch));
    ctrl->pressFavorite();
  }
  // Then: {6, 12, 37}만 남음
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_EQ(3u, favs.size());
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 6));
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 37));
}

TEST_F(ControllerTest, FavoriteAtChannel0) {
  // Given: 0번 채널 시청 중
  tuner->setCH("0");
  // When: 선호채널추가
  ctrl->pressFavorite();
  // Then: 0번이 선호 목록에 포함
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 0));
}

TEST_F(ControllerTest, FavoriteAdd_TwoChannels) {
  // Given: 4번, 56번 채널
  tuner->setCH("4");
  ctrl->pressFavorite();
  tuner->setCH("56");
  ctrl->pressFavorite();
  // Then: 두 채널 모두 선호 목록에 존재
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_EQ(2u, favs.size());
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 4));
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 56));
}

TEST_F(ControllerTest, FavoriteAtChannel99) {
  // Given: 99번 채널 시청 중
  tuner->setCH("99");
  // When: 선호채널추가
  ctrl->pressFavorite();
  // Then: 99번이 선호 목록에 포함
  const auto &favs = ctrl->getFavoriteChannels();
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 99));
}

// ─── FR-03: 다음 선호 채널 ───────────────────────────────────

TEST_F(ControllerTest, NextFavorite_Normal) {
  // Given: 선호 {1,4,12,56}, 현재 6번
  for (int ch : {1, 4, 12, 56})
    ctrl->addFavorite(ch);
  tuner->setCH("6");
  // When: 다음선호채널
  ctrl->pressNextFavorite();
  // Then: 12번 채널
  EXPECT_EQ(12, currentCh());
}

TEST_F(ControllerTest, NextFavorite_WrapAround) {
  // Given: 선호 {1,56}, 현재 56번
  ctrl->addFavorite(1);
  ctrl->addFavorite(56);
  tuner->setCH("56");
  // When: 다음선호채널
  ctrl->pressNextFavorite();
  // Then: 로테이션하여 1번
  EXPECT_EQ(1, currentCh());
}

TEST_F(ControllerTest, NextFavorite_EmptyList) {
  // Given: 선호 목록 비어 있음, 현재 6번
  tuner->setCH("6");
  // When: 다음선호채널
  ctrl->pressNextFavorite();
  // Then: 채널 변화 없음
  EXPECT_EQ(6, currentCh());
}

TEST_F(ControllerTest, NextFavorite_ExactMatchCurrent) {
  // Given: 선호 {1,4,12,56}, 현재 12번(선호와 동일)
  for (int ch : {1, 4, 12, 56})
    ctrl->addFavorite(ch);
  tuner->setCH("12");
  // When: 다음선호채널
  ctrl->pressNextFavorite();
  // Then: 12보다 큰 최소값 56
  EXPECT_EQ(56, currentCh());
}

TEST_F(ControllerTest, NextFavorite_SingleFavoriteWrap) {
  // Given: 선호 {7}만 존재, 현재 7번
  ctrl->addFavorite(7);
  tuner->setCH("7");
  // When: 다음선호채널
  ctrl->pressNextFavorite();
  // Then: wrap하여 7 유지
  EXPECT_EQ(7, currentCh());
}

TEST_F(ControllerTest, NextFavorite_FromChannel0) {
  // Given: 선호 {1,4,12}, 현재 0번
  for (int ch : {1, 4, 12})
    ctrl->addFavorite(ch);
  tuner->setCH("0");
  // When: 다음선호채널
  ctrl->pressNextFavorite();
  // Then: 0보다 큰 최소값 1
  EXPECT_EQ(1, currentCh());
}

// ─── FR-04: 채널 검색 ───────────────────────────────────────

TEST_F(ControllerTest, ChannelSearch_StoresAllFromFakeTuner) {
  // Given: FakeTuner 시청 가능 {1,4,12,56}, 현재 0번
  // When: 채널검색
  ctrl->pressChannelSearch();
  // Then: 검색 결과에 4개 채널 저장
  const auto &found = ctrl->getSearchedChannels();
  ASSERT_EQ(4u, found.size());
  EXPECT_EQ(1, found[0]);
  EXPECT_EQ(4, found[1]);
  EXPECT_EQ(12, found[2]);
  EXPECT_EQ(56, found[3]);
}

TEST_F(ControllerTest, ChannelSearch_EnablesListBasedUpDown) {
  // Given: 채널 검색 완료, 현재 6번
  ctrl->pressChannelSearch();
  tuner->setCH("6");
  // When: 채널 업
  ctrl->pressChannelUp();
  // Then: 검색 목록 내 다음 12
  EXPECT_EQ(12, currentCh());
}

TEST_F(ControllerTest, ChannelSearch_SecondSearchReplacesList) {
  // Given: 첫 검색 후 채널 변경
  ctrl->pressChannelSearch();
  tuner->setCH("56");
  // When: 재검색
  ctrl->pressChannelSearch();
  // Then: 동일 4채널 목록 유지
  EXPECT_EQ(4u, ctrl->getSearchedChannels().size());
}

TEST_F(ControllerTest, ChannelSearch_FromMidChannel) {
  // Given: 12번에서 검색 시작
  tuner->setCH("12");
  // When: 채널검색
  ctrl->pressChannelSearch();
  // Then: 전체 목록 수집
  EXPECT_EQ(4u, ctrl->getSearchedChannels().size());
}

TEST_F(ChannelSearchMockTest, ChannelSearch_CallsSeekCHRepeatedly) {
  // Given: Mock Tuner, seekCH가 순환 반환
  ::testing::InSequence seq;
  EXPECT_CALL(mockTuner, seekCH())
      .WillOnce(::testing::Return("4"))
      .WillOnce(::testing::Return("6"))
      .WillOnce(::testing::Return("14"))
      .WillOnce(::testing::Return("4"));
  // When: 채널검색
  ctrl->pressChannelSearch();
  // Then: 3채널 저장 (첫 반환 후 순환 종료)
  const auto &found = ctrl->getSearchedChannels();
  ASSERT_EQ(3u, found.size());
  EXPECT_EQ(4, found[0]);
  EXPECT_EQ(6, found[1]);
  EXPECT_EQ(14, found[2]);
}

TEST_F(ChannelSearchMockTest, ChannelSearch_SingleChannelLoop) {
  // Given: seekCH가 동일 채널만 반환
  EXPECT_CALL(mockTuner, seekCH()).WillRepeatedly(::testing::Return("7"));
  // When: 채널검색
  ctrl->pressChannelSearch();
  // Then: 1개 채널만 저장
  ASSERT_EQ(1u, ctrl->getSearchedChannels().size());
  EXPECT_EQ(7, ctrl->getSearchedChannels()[0]);
}

// ─── FR-05: 채널 업 (검색 결과 없음) ─────────────────────────

TEST_F(ControllerTest, ChannelUp_NoSearch_From6_To7) {
  // Given: 검색 없음, 현재 6번
  tuner->setCH("6");
  // When: 채널 업
  ctrl->pressChannelUp();
  // Then: 7번
  EXPECT_EQ(7, currentCh());
}

TEST_F(ControllerTest, ChannelUp_NoSearch_Wrap99to0) {
  // Given: 검색 없음, 현재 99번
  tuner->setCH("99");
  // When: 채널 업
  ctrl->pressChannelUp();
  // Then: 0번으로 순환
  EXPECT_EQ(0, currentCh());
}

TEST_F(ControllerTest, ChannelUp_NoSearch_From0_To1) {
  // Given: 검색 없음, 현재 0번
  tuner->setCH("0");
  // When: 채널 업
  ctrl->pressChannelUp();
  // Then: 1번
  EXPECT_EQ(1, currentCh());
}

TEST_F(ControllerTest, ChannelUp_NoSearch_From98_To99) {
  // Given: 검색 없음, 현재 98번
  tuner->setCH("98");
  // When: 채널 업
  ctrl->pressChannelUp();
  // Then: 99번
  EXPECT_EQ(99, currentCh());
}

TEST_F(ControllerTest, ChannelUp_NoSearch_ConsecutivePress) {
  // Given: 검색 없음, 현재 5번
  tuner->setCH("5");
  // When: 채널 업 2회
  ctrl->pressChannelUp();
  ctrl->pressChannelUp();
  // Then: 7번
  EXPECT_EQ(7, currentCh());
}

// ─── FR-06: 채널 업 (검색 결과 있음) ─────────────────────────

TEST_F(ControllerSearchListTest, ChannelUp_WithSearch_OnList_6to14) {
  // Given: 검색 목록 {4,6,14}, 현재 6번
  searchAndSetChannel(6);
  // When: 채널 업
  ctrl->pressChannelUp();
  // Then: 14번
  EXPECT_EQ(14, currentCh());
}

TEST_F(ControllerSearchListTest, ChannelUp_WithSearch_OffList_15to4) {
  // Given: 검색 목록 {4,6,14}, 현재 15번(목록 외)
  searchAndSetChannel(15);
  // When: 채널 업
  ctrl->pressChannelUp();
  // Then: 목록 최소값 4
  EXPECT_EQ(4, currentCh());
}

TEST_F(ControllerSearchListTest, ChannelUp_WithSearch_AtMax_14to4) {
  // Given: 검색 목록 {4,6,14}, 현재 14번
  searchAndSetChannel(14);
  // When: 채널 업
  ctrl->pressChannelUp();
  // Then: wrap하여 4
  EXPECT_EQ(4, currentCh());
}

// ─── FR-05: 채널 다운 (검색 결과 없음) ───────────────────────

TEST_F(ControllerTest, ChannelDown_NoSearch_From6_To5) {
  // Given: 검색 없음, 현재 6번
  tuner->setCH("6");
  // When: 채널 다운
  ctrl->pressChannelDown();
  // Then: 5번
  EXPECT_EQ(5, currentCh());
}

TEST_F(ControllerTest, ChannelDown_NoSearch_Wrap0to99) {
  // Given: 검색 없음, 현재 0번
  tuner->setCH("0");
  // When: 채널 다운
  ctrl->pressChannelDown();
  // Then: 99번으로 순환
  EXPECT_EQ(99, currentCh());
}

TEST_F(ControllerTest, ChannelDown_NoSearch_From1_To0) {
  // Given: 검색 없음, 현재 1번
  tuner->setCH("1");
  // When: 채널 다운
  ctrl->pressChannelDown();
  // Then: 0번
  EXPECT_EQ(0, currentCh());
}

TEST_F(ControllerTest, ChannelDown_NoSearch_From99_To98) {
  // Given: 검색 없음, 현재 99번
  tuner->setCH("99");
  // When: 채널 다운
  ctrl->pressChannelDown();
  // Then: 98번
  EXPECT_EQ(98, currentCh());
}

TEST_F(ControllerTest, ChannelDown_NoSearch_ConsecutivePress) {
  // Given: 검색 없음, 현재 10번
  tuner->setCH("10");
  // When: 채널 다운 2회
  ctrl->pressChannelDown();
  ctrl->pressChannelDown();
  // Then: 8번
  EXPECT_EQ(8, currentCh());
}

// ─── FR-06: 채널 다운 (검색 결과 있음) ───────────────────────

TEST_F(ControllerSearchListTest, ChannelDown_WithSearch_OnList_6to4) {
  // Given: 검색 목록 {4,6,14}, 현재 6번
  searchAndSetChannel(6);
  // When: 채널 다운
  ctrl->pressChannelDown();
  // Then: 4번
  EXPECT_EQ(4, currentCh());
}

TEST_F(ControllerSearchListTest, ChannelDown_WithSearch_OffList_15to14) {
  // Given: 검색 목록 {4,6,14}, 현재 15번
  searchAndSetChannel(15);
  // When: 채널 다운
  ctrl->pressChannelDown();
  // Then: 15 미만 최대값 14
  EXPECT_EQ(14, currentCh());
}

TEST_F(ControllerSearchListTest, ChannelDown_WithSearch_AtMin_4to14) {
  // Given: 검색 목록 {4,6,14}, 현재 4번(목록 최소)
  searchAndSetChannel(4);
  // When: 채널 다운
  ctrl->pressChannelDown();
  // Then: wrap하여 14
  EXPECT_EQ(14, currentCh());
}

TEST_F(ControllerSearchListTest, ChannelDown_WithSearch_On14_To6) {
  // Given: 검색 목록, 현재 14번
  searchAndSetChannel(14);
  // When: 채널 다운
  ctrl->pressChannelDown();
  // Then: 6번
  EXPECT_EQ(6, currentCh());
}

TEST_F(ControllerSearchListTest, ChannelUpDown_WithSearch_FullCycle) {
  // Given: 검색 목록, 현재 6번
  searchAndSetChannel(6);
  // When: 업 후 다운
  ctrl->pressChannelUp();
  ASSERT_EQ(14, currentCh());
  ctrl->pressChannelDown();
  // Then: 6번으로 복귀
  EXPECT_EQ(6, currentCh());
}
