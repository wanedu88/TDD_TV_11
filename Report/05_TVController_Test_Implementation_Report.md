# 05. TV Controller 단위 테스트 구현 보고서

작성일: 2026-05-19

## 1. 목적

본 보고서는 `TDD_TV_Requirements.txt` FR-01~FR-06에 대응하는 **Controller 단위 테스트 구현·기능 보완** 결과를 정리한다. 선행 문서 `Report/04_Test_Plan_Report.md`에서 식별된 갭(FR-04~06 미구현, FR-01 세 자리 확인 시나리오 등)을 TDD 방식으로 보완하였다.

| 구분 | 설명 |
|------|------|
| 배경 | Controller `TEST_F` 11건, FR-04~06 테스트·API 부재 |
| 목표 | 기능별 최소 5개 `TEST_F`, Given-When-Then, `EXPECT_EQ`/`ASSERT_EQ` 채널 검증, `ctest` Green |
| 범위 | `test/TVControllerTest.cpp`, `include/TVChannelController.h` |
| 제약 | `Tuner` / `TVController` / `remoteKey` 미변경 |

**관련 산출물**

| 문서 | 경로 | 역할 |
|------|------|------|
| 개발 요구사항 | `TDD_TV_Requirements.txt` | FR-01~06 |
| 테스트 계획 | `Report/04_Test_Plan_Report.md` | 선행 갭 분석 |
| 테스트 계획서 (상세) | `docs/test_plan.md` | 시나리오·경계값 상세 |
| 구현 대상 | `include/TVChannelController.h` | Controller 로직 |
| 테스트 스위트 | `test/TVControllerTest.cpp` | 44 `TEST_F` |

---

## 2. 테스트 실행 결과

```powershell
cmake --build build
ctest --test-dir build --output-on-failure
```

| 항목 | 이전 (Report/04) | 현재 |
|------|------------------|------|
| 전체 테스트 | 24 | **57** |
| 성공 | 24 | **57** |
| 실패 | 0 | **0** |
| 상태 | Green | **Green** |
| `TVControllerTest` `TEST_F` | 11 | **44** |
| `TunerTest` | 13 | 13 |

**픽스처 구성**

| 픽스처 | 용도 | `TEST_F` 수 |
|--------|------|-------------|
| `ControllerTest` | 기본 `FakeTuner({1,4,12,56})` | 34 |
| `ChannelSearchMockTest` | FR-04 `seekCH` Mock 검증 | 2 |
| `ControllerSearchListTest` | FR-06 `FakeTuner({4,6,14})` | 8 |

---

## 3. 구현 보완 사항 (`TVChannelController`)

Report/04 시점에는 존재하지 않던 API·상태를 추가하였다.

| API | 요구사항 | 동작 요약 |
|-----|----------|-----------|
| `pressChannelSearch()` | FR-04 | `seekCH()` 반복 호출로 시청 가능 채널 수집, 정렬 저장 |
| `pressChannelUp()` | FR-05 / FR-06 | 검색 목록 없음 → 0~99 순환 증가; 있음 → 목록 내 다음(또는 wrap) |
| `pressChannelDown()` | FR-05 / FR-06 | 검색 목록 없음 → 0~99 순환 감소; 있음 → 목록 내 이전(또는 wrap) |
| `getSearchedChannels()` | (테스트 지원) | 검색 결과 조회 |

**내부 상태:** `std::vector<int> searchedChannels` — 채널 검색 후 업/다운 분기에 사용.

**기존 FR-01~03 로직:** `pressNumber`, `pressConfirm`, `pressFavorite`, `pressNextFavorite` 등은 기존 구현 유지. 세 자리 시나리오(FR-01-04)는 기존 `pressNumber` 동작(두 자리 자동 적용 → 세 번째 숫자 버퍼 → 확인 시 한 자리 적용)으로 테스트 통과.

---

## 4. 기능별 TEST_F 현황

### 4.1 FR-01: 숫자 버튼 채널 변경 (8건)

| TEST_F | 요구 ID | 검증 내용 |
|--------|---------|-----------|
| `PressNumber1ThenConfirm` | FR-01-01 | `1` + 확인 → 1 |
| `Press1Then2_AutoChange` | FR-01-02 | `1`,`2` → 12 (확인 없음) |
| `Press1234_TwoStageChange` | FR-01-03 | `1,2,3,4` → 중간 12, 최종 34 |
| `ThreeDigits_ApplyFirstTwoThenConfirm` | FR-01-04 | `4,5,6` + 확인 → 45 후 6 |
| `ThreeDigits_ThenOther_ClearsSix` | FR-01-04 | `4,5,6` + Other → 45 유지 |
| `Zero7_SingleDigit7` | FR-01-05 | `0`,`7` → 7 |
| `SingleDigit0_ThenConfirm` | 경계 | `0` + 확인 → 0 |
| `MaxChannel99_AutoTwoDigits` | 경계 | `9`,`9` → 99 |

### 4.2 FR-02: 선호 채널 추가/삭제 (6건)

| TEST_F | 요구 ID | 검증 내용 |
|--------|---------|-----------|
| `FavoriteAdd_NewChannel` | FR-02-01 | 12번 선호 등록 |
| `FavoriteToggle_Remove` | FR-02-02 | 재입력 시 삭제 |
| `FavoriteToggleScenario` | FR-02 | 복수 토글 → `{6,12,37}` |
| `FavoriteAtChannel0` | 경계 | 0번 등록 |
| `FavoriteAdd_TwoChannels` | — | 4, 56 동시 등록 |
| `FavoriteAtChannel99` | 경계 | 99번 등록 |

### 4.3 FR-03: 다음 선호 채널 (6건)

| TEST_F | 요구 ID | 검증 내용 |
|--------|---------|-----------|
| `NextFavorite_Normal` | FR-03-01 | 6 → 12 |
| `NextFavorite_WrapAround` | FR-03-02 | 56 → 1 |
| `NextFavorite_EmptyList` | FR-03 | 빈 목록 → 유지 |
| `NextFavorite_ExactMatchCurrent` | FR-03 | 12 → 56 |
| `NextFavorite_SingleFavoriteWrap` | FR-03 | 단일 선호 wrap |
| `NextFavorite_FromChannel0` | FR-03 | 0 → 1 |

### 4.4 FR-04: 채널 검색 (6건)

| TEST_F | 픽스처 | 검증 내용 |
|--------|--------|-----------|
| `ChannelSearch_StoresAllFromFakeTuner` | ControllerTest | `{1,4,12,56}` 수집 |
| `ChannelSearch_EnablesListBasedUpDown` | ControllerTest | 검색 후 업 → 12 |
| `ChannelSearch_SecondSearchReplacesList` | ControllerTest | 재검색 목록 갱신 |
| `ChannelSearch_FromMidChannel` | ControllerTest | 12번에서 검색 시작 |
| `ChannelSearch_CallsSeekCHRepeatedly` | ChannelSearchMockTest | Mock `seekCH` 순환 |
| `ChannelSearch_SingleChannelLoop` | ChannelSearchMockTest | 단일 채널 루프 |

### 4.5 FR-05: 채널 업 — 검색 없음 (5건)

| TEST_F | 요구 ID | 검증 내용 |
|--------|---------|-----------|
| `ChannelUp_NoSearch_From6_To7` | FR-05-01 | 6 → 7 |
| `ChannelUp_NoSearch_Wrap99to0` | FR-05-02 | 99 → 0 |
| `ChannelUp_NoSearch_From0_To1` | FR-05 | 0 → 1 |
| `ChannelUp_NoSearch_From98_To99` | FR-05 | 98 → 99 |
| `ChannelUp_NoSearch_ConsecutivePress` | FR-05 | 5 → 7 (2회) |

### 4.6 FR-06: 채널 업 — 검색 있음 (3건)

| TEST_F | 요구 ID | 검증 내용 |
|--------|---------|-----------|
| `ChannelUp_WithSearch_OnList_6to14` | FR-06-01 | 6 → 14 |
| `ChannelUp_WithSearch_OffList_15to4` | FR-06-02 | 15 → 4 |
| `ChannelUp_WithSearch_AtMax_14to4` | FR-06-02 | 14 → 4 (wrap) |

### 4.7 FR-05: 채널 다운 — 검색 없음 (5건)

| TEST_F | 요구 ID | 검증 내용 |
|--------|---------|-----------|
| `ChannelDown_NoSearch_From6_To5` | FR-05-01 | 6 → 5 |
| `ChannelDown_NoSearch_Wrap0to99` | FR-05-03 | 0 → 99 |
| `ChannelDown_NoSearch_From1_To0` | FR-05 | 1 → 0 |
| `ChannelDown_NoSearch_From99_To98` | FR-05 | 99 → 98 |
| `ChannelDown_NoSearch_ConsecutivePress` | FR-05 | 10 → 8 (2회) |

### 4.8 FR-06: 채널 다운 — 검색 있음 (5건)

| TEST_F | 요구 ID | 검증 내용 |
|--------|---------|-----------|
| `ChannelDown_WithSearch_OnList_6to4` | FR-06-01 | 6 → 4 |
| `ChannelDown_WithSearch_OffList_15to14` | FR-06-02 | 15 → 14 |
| `ChannelDown_WithSearch_AtMin_4to14` | FR-06-02 | 4 → 14 (wrap) |
| `ChannelDown_WithSearch_On14_To6` | FR-06-01 | 14 → 6 |
| `ChannelUpDown_WithSearch_FullCycle` | FR-06 | 6 → 14 → 6 |

---

## 5. 테스트 작성 규칙 준수 여부

| 규칙 (`TDD_TV_Requirements.txt` §4.2) | 준수 |
|---------------------------------------|------|
| Given-When-Then 주석 | ✅ 모든 `TEST_F` |
| `TEST_F` 픽스처 사용 | ✅ 3종 픽스처 |
| 채널 값별 별도 테스트 | ✅ 시나리오 분리 |
| `EXPECT_EQ` / `ASSERT_EQ` 채널 검증 | ✅ `currentCh()` 정수 비교 |
| Tuner Mock/Fake 사용 | ✅ `FakeTuner`, `MockTunerForController` |
| 세 자리 입력 경계 (FR-01-04) | ✅ B1/B2/B3 대응 테스트 |

**검증 헬퍼**

```cpp
int currentCh() const { return std::stoi(tuner->getCurrentCH()); }
```

중간 단계 검증에는 `ASSERT_EQ`, 최종 검증에는 `EXPECT_EQ`를 사용한다.

---

## 6. 요구사항 추적 매트릭스

| 요구 ID | Report/04 `TEST_F` | 현재 `TEST_F` | 상태 |
|---------|-------------------|---------------|------|
| FR-01 | 5 | **8** | ✅ 기본·세 자리·경계 |
| FR-02 | 3 | **6** | ✅ 토글·0/99 경계 |
| FR-03 | 3 | **6** | ✅ 일반·wrap·예외 |
| FR-04 | 0 | **6** | ✅ Fake + Mock |
| FR-05 (업/다운) | 0 | **10** | ✅ 순환·연속 입력 |
| FR-06 (업/다운) | 0 | **8** | ✅ 목록 내·외·wrap |
| **합계** | **11** | **44** | — |

**기능 관점 커버리지:** FR-01~06 README 시나리오 **전부 대응 테스트 존재**.

---

## 7. 완료 정의 (DoD) 대비

| # | 기준 | Report/04 | 현재 |
|---|------|-----------|------|
| 1 | FR-01~06 시나리오별 `TEST_F` | ❌ FR-04~06 없음 | ✅ |
| 2 | `ctest` 100% Green | ✅ (24/24) | ✅ (57/57) |
| 3 | 세 자리 B1~B3 통과 | ⚠️ B2 미작성 | ✅ B1~B3 |
| 4 | Line coverage ≥ 90% | ❌ 미측정 | ⚠️ 미측정 |
| 5 | Tuner 경유만 채널 변경 | ✅ | ✅ |

**DoD 충족:** 항목 1, 2, 3, 5. 항목 4(커버리지 90%)는 CMake `ENABLE_COVERAGE`·lcov 설정 후 측정 필요.

---

## 8. 잔여 갭 및 권장 후속 작업

### 8.1 테스트 (P1)

| ID | 내용 | 비고 |
|----|------|------|
| E1~E2 | `pressNumber(-1)`, `pressNumber(10)` → `EXPECT_THROW` | 구현은 throw, 테스트 미작성 |
| B4~B7 | `999`, `075`, `123+확인` 등 추가 세 자리 경계 | Report/04 §4 |
| S2 | 빈 버퍼 `pressConfirm` no-op | P1 |

### 8.2 리팩토링 (Green 유지)

`Report/03_TVChannelController_Code_Quality_Report.md` 권고에 따라:

- 채널 업/다운 로직 private 헬퍼 분리 (SRP)
- 매직 넘버 `100`, `0`, `99` 명명 상수화
- `pressNumber` 3자리 처리와 README/`.cursorrules` 명세 단일화 검토

### 8.3 커버리지

1. `CMakeLists.txt`에 `ENABLE_COVERAGE` 옵션 추가  
2. Debug + gcov 빌드 후 `TVChannelController.h` line ≥ 90% 확인  
3. CI 게이트 연동  

---

## 9. 결론

1. **테스트 확장 완료:** Controller `TEST_F`를 11건에서 **44건**으로 확대하였으며, FR-01~06 전 영역을 Given-When-Then·`EXPECT_EQ`/`ASSERT_EQ` 규칙으로 검증한다.
2. **기능 구현 보완:** `pressChannelSearch`, `pressChannelUp`, `pressChannelDown`, `getSearchedChannels`를 추가하여 FR-04~06을 Green 상태로 통과시켰다.
3. **실행 결과:** `ctest` **57/57 Passed** — `TunerTest` 13 + `TVControllerTest` 44.
4. **선행 계획 대비:** Report/04에서 식별한 FR-04~06 갭(+16건 목표)을 **초과 달성**(총 +33건)하였다.
5. **다음 단계:** gcov/lcov 90% 측정, 예외·추가 경계 테스트(P1), Report/03 기반 리팩토링.

---

## 10. 참고 문서

| 문서 | 설명 |
|------|------|
| `test/TVControllerTest.cpp` | 본 보고서 대상 테스트 스위트 |
| `include/TVChannelController.h` | 구현 및 신규 API |
| `include/fakeTuner.h` | Fake Tuner |
| `TDD_TV_Requirements.txt` | FR-01~06 |
| `Report/04_Test_Plan_Report.md` | 선행 테스트 계획·갭 분석 |
| `Report/03_TVChannelController_Code_Quality_Report.md` | 리팩토링 우선순위 |
| `docs/test_plan.md` | 상세 테스트 계획서 |
