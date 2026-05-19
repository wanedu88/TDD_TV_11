# 06. TVChannelController 결함 분석 보고서

작성일: 2026-05-19

## 1. 목적

본 보고서는 `test/TVControllerTest.cpp` 기준 **ctest 실패 로그 분석**, `TVChannelController` **함수별 결함 위치 특정**, **심각도 분류**, **최소 변경 수정 방안**을 정리한다.

| 구분 | 설명 |
|------|------|
| 역할 | C++ QA — 디버깅·결함 분석 |
| 분석 대상 | `include/TVChannelController.h`, `test/TVControllerTest.cpp` |
| 제약 | `Tuner`, `TVController`, `remoteKey` **수정 금지** |
| 검증 | C++17, `cmake --build build` + `ctest` Green |

**관련 산출물**

| 문서 | 경로 | 역할 |
|------|------|------|
| 개발 요구사항 | `TDD_TV_Requirements.txt` | FR-01~06 |
| 테스트 스위트 | `test/TVControllerTest.cpp` | 44 `TEST_F` |
| 구현 대상 | `include/TVChannelController.h` | Controller 로직 |
| 선행 보고서 | `Report/05_TVController_Test_Implementation_Report.md` | 테스트·API 보완 결과 |
| 코드 품질 | `Report/03_TVChannelController_Code_Quality_Report.md` | 리팩토링 관점 |

---

## 2. 테스트 실행 현황

```powershell
cd c:\DEV\TDD_TV_11
cmake --build build
ctest --test-dir build --output-on-failure
```

| 항목 | 값 |
|------|-----|
| 전체 테스트 | **57** |
| 성공 | **57** |
| 실패 | **0** |
| 상태 | **Green** |
| `TVControllerTest` `TEST_F` | **44** |

**분석 방법:** 과거 커밋(`815a16d` `pressOther` 버그) 및 FR-04~06 스텁 재현으로 실패 로그를 수집한 뒤, 현재 HEAD 구현과 대조하였다.

---

## 3. EXPECT_EQ 실패 — 기대/실제 차이 요약

### 3.1 FR-01 — `pressOther()` 오동작 (1건)

| 테스트 | 파일:줄 | 기대 | 실제 | 차이 |
|--------|---------|------|------|------|
| `ThreeDigits_ThenOther_ClearsSix` | `test/TVControllerTest.cpp:108` | `45` | `0` | **-45** |

**재현 로그 (요약):**

```
C:\DEV\TDD_TV_11\test\TVControllerTest.cpp:108: Failure
Expected equality of these values:
  45
  currentCh()
    Which is: 0
```

**원인:** `4,5,6` 입력 후 채널은 45인데, `pressOther()`가 버퍼 클리어 후 `tuner_.setCH("0")`까지 호출 (커밋 `815a16d`).

---

### 3.2 FR-04~06 미구현 / 스텁 (24건)

| 영역 | 대표 테스트 | 파일:줄 | 기대 | 실제 | 패턴 |
|------|-------------|---------|------|------|------|
| FR-04 검색 | `ChannelSearch_StoresAllFromFakeTuner` | :281 | `found.size() == 4` | `0` | 검색 목록 미수집 |
| FR-05 업 | `ChannelUp_NoSearch_From6_To7` | :353 | `7` | `6` | 채널 무변경 |
| FR-05 다운 | `ChannelDown_NoSearch_From6_To5` | :430 | `5` | `6` | 동일 |
| FR-05 wrap | `ChannelUp_NoSearch_Wrap99to0` | :362 | `0` | `99` | 0~99 순환 미구현 |
| FR-06 목록 업 | `ChannelUp_WithSearch_OnList_6to14` | :401 | `14` | `6` | 검색 목록 기반 이동 없음 |
| FR-06 목록 다운 | `ChannelDown_WithSearch_OnList_6to4` | :478 | `4` | `6` | 동일 |
| Mock FR-04 | `ChannelSearch_CallsSeekCHRepeatedly` | :329 | `found.size() == 3` | `0` | `seekCH()` 미호출 |

**재현 로그 예 (FR-04):**

```
C:\DEV\TDD_TV_11\test\TVControllerTest.cpp:281: Failure
Expected equality of these values:
  4u
    Which is: 4
  found.size()
    Which is: 0
```

**재현 로그 예 (FR-05):**

```
C:\DEV\TDD_TV_11\test\TVControllerTest.cpp:353: Failure
Expected equality of these values:
  7
  currentCh()
    Which is: 6
```

**공통 패턴:** `pressChannelSearch` / `pressChannelUp` / `pressChannelDown` 이 no-op이면 채널·목록 검증이 **기대 ≠ 현재(초기값 유지)** 로 실패한다.

---

### 3.3 FR-01~03 — 통과 (참고)

`PressNumber1ThenConfirm`, `Press1Then2_AutoChange`, `NextFavorite_*`, `Favorite*` 등 **20건**은 현재 구현에서 통과한다.

---

## 4. 함수별 버그 위치 (`include/TVChannelController.h`)

| 함수 | 줄 | 결함 | 연관 FR / 테스트 |
|------|-----|------|------------------|
| `pressOther()` | **54** | (과거) `buffer.clear()` 후 `setCH("0")` 호출 | FR-01-04c → `ThreeDigits_ThenOther_ClearsSix` |
| `pressNumber()` | **40–49** | 3번째 숫자 시 `buffer.clear()`만 수행 — 2자리 자동 적용과 조합 시 명세 민감 | FR-01-04 → `ThreeDigits_ApplyFirstTwoThenConfirm` |
| `pressChannelSearch()` | **92–105** | (과거) 미구현 → `searchedChannels` 비어 있음 | FR-04 → 6 tests |
| `pressChannelUp()` | **107–121** | (과거) 미구현 → `(ch+1)%100`·목록 탐색 없음 | FR-05/06 → 11 tests |
| `pressChannelDown()` | **123–150** | (과거) 미구현 → `(ch-1+100)%100`·목록 탐색 없음 | FR-05/06 → 13 tests |
| `applyBuffer()` | 16–28 | 정상 (FR-01 핵심) | — |
| `pressConfirm()` | 52 | 정상 | FR-01 |
| `pressFavorite()` | 57–67 | 정상 | FR-02 |
| `pressNextFavorite()` | 76–89 | 정상 (`upper_bound` + wrap) | FR-03 |
| `getSearchedChannels()` | 152 | (과거) 빈 벡터 반환 | FR-04 테스트 지원 |

---

## 5. 결함 심각도 분류

| ID | 심각도 | 함수 | 근거 |
|----|--------|------|------|
| **D-01** | **Critical** | `pressChannelSearch`, `pressChannelUp`, `pressChannelDown` | FR-04~06 전면 미동작. 24 tests 실패, README·요구사항 필수 기능 누락 |
| **D-02** | **Major** | `pressOther()` (과거) | FR-01-04c 위반: 45 유지해야 하나 0으로 변경. 사용자 채널 의도치 않 변경 |
| **D-03** | **Minor** | `pressNumber()` 3자리 분기 | 명세·테스트 정합 시 버퍼 잔류 로직 민감. 현재는 2자리 자동 적용 후 3번째는 단일 버퍼로 동작해 Green |
| **D-04** | **Info** | `getFavoriteChannels()` | 매 호출 `vector` 복사 — 기능 결함 아님, 성능·API 스타일 이슈 (`Report/03` 참조) |

---

## 6. 최소 변경 수정 방안 (C++17)

수정 범위: **`include/TVChannelController.h`만**. `Tuner` / `TVController` / `remoteKey` 미변경.

### 6.1 D-02: `pressOther()` — 버퍼만 클리어

```diff
--- a/include/TVChannelController.h
+++ b/include/TVChannelController.h
@@ -51,9 +51,7 @@ public:
   void pressConfirm() { applyBuffer(); }
 
-  void pressOther() {
-    buffer.clear();
-    tuner_.setCH("0");
-  }
+  void pressOther() { buffer.clear(); }
```

**요구사항:** FR-01-04 — `4,5,6` 후 Other → **45 유지**, 잔여 `6`만 무효화.

---

### 6.2 D-01: FR-04~06 API 추가

**추가 멤버:** `std::vector<int> searchedChannels`

| API | 요구사항 | 동작 요약 |
|-----|----------|-----------|
| `pressChannelSearch()` | FR-04 | `seekCH()` 반복 → 첫 값 순환 종료 → 정렬 저장 |
| `pressChannelUp()` | FR-05 / FR-06 | 검색 없음: `(ch+1)%100` / 있음: `upper_bound` 다음 또는 wrap |
| `pressChannelDown()` | FR-05 / FR-06 | 검색 없음: `(ch-1+100)%100` / 있음: `lower_bound` 이전 또는 wrap |
| `getSearchedChannels()` | (테스트) | 검색 결과 const 참조 반환 |

**핵심 구현 (현재 HEAD):**

```cpp
void pressChannelSearch() {
  searchedChannels.clear();
  std::string ch = tuner_.seekCH();
  int firstVal = std::stoi(ch);
  searchedChannels.push_back(firstVal);
  while (true) {
    ch = tuner_.seekCH();
    int val = std::stoi(ch);
    if (val == firstVal)
      break;
    searchedChannels.push_back(val);
  }
  std::sort(searchedChannels.begin(), searchedChannels.end());
}

void pressChannelUp() {
  if (searchedChannels.empty()) {
    int ch = std::stoi(tuner_.getCurrentCH());
    tuner_.setCH(std::to_string((ch + 1) % 100));
    return;
  }
  int current = std::stoi(tuner_.getCurrentCH());
  auto it = std::upper_bound(searchedChannels.begin(), searchedChannels.end(), current);
  tuner_.setCH(std::to_string(it == searchedChannels.end() ? searchedChannels.front() : *it));
}

void pressChannelDown() {
  if (searchedChannels.empty()) {
    int ch = std::stoi(tuner_.getCurrentCH());
    tuner_.setCH(std::to_string((ch - 1 + 100) % 100));
    return;
  }
  // 목록 내 이전 / wrap — lower_bound + upper_bound 분기 (123~150행)
}
```

---

### 6.3 D-03: `pressNumber()` — 세 자리 입력 (현재 Green)

| 단계 | 동작 |
|------|------|
| `4`, `5` | 2자리 완성 → `applyBuffer()` → 채널 **45** |
| `6` | 버퍼 `[6]`만 유지 (3자리 clear 분기 미진입) |
| `확인` | `applyBuffer()` → 채널 **6** |
| `Other` | `buffer.clear()`만 — 채널 **45** 유지 |

---

## 7. 실패 테스트 ↔ 결함 매핑

| 실패 건수 | 결함 ID | 영향 받는 `TEST_F` (요약) |
|-----------|---------|---------------------------|
| 1 | D-02 | `ThreeDigits_ThenOther_ClearsSix` |
| 6 | D-01 | `ChannelSearch_*`, Mock 검색 2건 |
| 10 | D-01 | `ChannelUp_NoSearch_*`, `ChannelUp_WithSearch_*` |
| 8 | D-01 | `ChannelDown_NoSearch_*`, `ChannelDown_WithSearch_*` |

---

## 8. Green 확인 절차

```powershell
cd c:\DEV\TDD_TV_11
cmake --build build
ctest --test-dir build --output-on-failure
```

**기대 결과:**

```
100% tests passed, 0 tests failed out of 57
```

**Controller만 빠른 확인:**

```powershell
.\build\TVControllerTest.exe
```

**특정 시나리오만:**

```powershell
.\build\TVControllerTest.exe --gtest_filter=ControllerTest.ThreeDigits_ThenOther_ClearsSix
.\build\TVControllerTest.exe --gtest_filter=ControllerTest.ChannelSearch_StoresAllFromFakeTuner
```

---

## 9. 결론

| 항목 | 내용 |
|------|------|
| 핵심 결함 | (1) `pressOther`의 불필요한 `setCH("0")` (2) FR-04~06 API 부재 |
| 수정 범위 | `include/TVChannelController.h` 만 |
| 현재 상태 | 위 수정 반영 완료, **ctest 57/57 Green** |
| 후속 | `Report/03` 리팩토링 우선순위(채널 값 객체, 버퍼 분리)는 Green 유지하며 단계 적용 |

---

## 10. Git 이력 참고

| 커밋 | 내용 |
|------|------|
| `815a16d` | `pressOther()`에 `setCH("0")` 추가 → `ThreeDigits_ThenOther` 실패 |
| `61c68e6` | `setCH("0")` 제거, 테스트 기대값 `45`로 수정 → Green |
| `df4a393` | FR-01~06 테스트 및 채널 검색·업/다운 API 구현 |

---

## 11. 부록 — 결함 목록 문서화 (추가 작업, 2026-05-19)

본 절은 보고서 초안(§1~10) 작성 **이후** QA 리드 요청으로 수행한 **결함 목록 정리**만 기록한다.

### 11.1 작업 요청

| 구분 | 내용 |
|------|------|
| 역할 | QA Lead |
| 목표 | 발견된 테스트 실패·결함을 표준 필드로 문서화 |
| 산출물 | `docs/defect_list.md` |
| 항목 형식 | `[ID]` `[Severity]` `[ItemType]` `[Steps]` `[Expected]` `[Actual]` `[Root Cause]` `[Fix Summary]` |

### 11.2 산출물 요약 (`docs/defect_list.md`)

| 항목 | 내용 |
|------|------|
| 경로 | `docs/defect_list.md` |
| 등록 결함 | **DEF-001 ~ DEF-014** (14건) |
| 출처 | §3~§5 본 보고서, `Report/01`·`04`·`05`, Git `815a16d` / `61c68e6` / `df4a393` |

**상태·유형 범례**

| 범주 | 값 |
|------|-----|
| Status | `Fixed` / `Open` / `Waived` |
| ItemType | `Bug`, `Missing Implementation`, `Specification Gap`, `Test Gap`, `Technical Debt` |

### 11.3 결함 등록 현황

| ID | Severity | ItemType | Status | 요약 |
|----|----------|----------|--------|------|
| DEF-001 | Critical | Missing Implementation | Fixed | FR-04 `pressChannelSearch` 미구현 |
| DEF-002 | Critical | Missing Implementation | Fixed | FR-05 `pressChannelUp` 미구현 |
| DEF-003 | Critical | Missing Implementation | Fixed | FR-06 `pressChannelDown` 미구현 |
| DEF-004 | Major | Bug | Fixed | `pressOther` → `setCH("0")` |
| DEF-005 | Minor | Bug | Fixed | 세 자리 입력 버퍼·확인 처리 |
| DEF-006 | Minor | Specification Gap | Open | FR-01-04 vs `.cursorrules` 3자리 규칙 |
| DEF-007 | Minor | Specification Gap | Open | 채널 두 자리 문자열 표기 |
| DEF-008 | Info | Technical Debt | Open | `getFavoriteChannels()` vector 복사 |
| DEF-009 | Minor | Technical Debt | Open | `applyBuffer` 0~99 검증 없음 |
| DEF-010 | Minor | Test Gap | Open | `pressNumber` 예외 E1~E2 미테스트 |
| DEF-011 | Minor | Test Gap | Open | 세 자리 경계 B4~B7 미작성 |
| DEF-012 | Info | Test Gap | Open | Line coverage 90% 미측정 |
| DEF-013 | Info | Technical Debt | Open (Waived) | `TVController::pushButton` 레거시 (수정 금지) |
| DEF-014 | Minor | Test Gap | Open | 빈 버퍼 `pressConfirm` no-op 미테스트 |

**Fixed 5건 / Open·Waived 9건** — 구현 수정(DEF-001~005)은 HEAD에서 **ctest 57/57 Green**과 일치.

### 11.4 본 보고서(§1~10)와의 관계

| 본 보고서 ID | defect_list ID | 비고 |
|--------------|----------------|------|
| D-01 | DEF-001, DEF-002, DEF-003 | FR-04~06을 항목별 분리 등록 |
| D-02 | DEF-004 | 동일 결함 |
| D-03 | DEF-005 | 동일 결함 |
| D-04 | DEF-008 | 동일 이슈, Test Gap·Technical Debt로 세분화는 DEF-010~014 |

§11은 **추적·이슈 관리용 상세 카드**이며, 분석·재현·수정 diff는 §1~10을 정본으로 한다.

### 11.5 관련 산출물 (동일 세션)

| 문서 | 경로 |
|------|------|
| 결함 목록 | `docs/defect_list.md` |
| 대화 transcript | `Prompting/06_TVChannelController_Defect_Analysis_prompt.md` |

---

*본 문서는 `test/TVControllerTest.cpp` 및 ctest 재현 로그 기준 정적·동적 분석 결과이다. §11은 결함 목록 문서화 추가 작업분이다.*
