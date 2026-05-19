# Defect List — TDD_TV_11

| 항목 | 내용 |
|------|------|
| 작성일 | 2026-05-19 |
| 작성 | QA Lead |
| 범위 | `TVChannelController`, `test/TVControllerTest.cpp` |
| 기준 | ctest 실패 재현, `Report/01~06`, Git `815a16d` / `61c68e6` / `df4a393` |
| 현재 테스트 | **57/57 Green** (HEAD) |

**상태 범례:** `Fixed` — 수정·검증 완료 | `Open` — 미해결 또는 후속 작업 | `Waived` — 명세/범위상 수용

**ItemType:** `Bug` | `Missing Implementation` | `Specification Gap` | `Test Gap` | `Technical Debt`

---

## DEF-001

| 필드 | 내용 |
|------|------|
| **ID** | DEF-001 |
| **Severity** | Critical |
| **ItemType** | Missing Implementation |
| **Status** | Fixed (`df4a393`) |
| **Steps** | 1. `FakeTuner({1,4,12,56})`로 `TVChannelController` 생성<br>2. `pressChannelSearch()` 호출<br>3. `getSearchedChannels().size()` 확인 |
| **Expected** | 시청 가능 채널 4개 `{1,4,12,56}` 저장 (`ChannelSearch_StoresAllFromFakeTuner`) |
| **Actual** | `found.size() == 0` (API 미구현·no-op 시) |
| **Root Cause** | `pressChannelSearch()` 및 `searchedChannels` 멤버 부재. `tuner_.seekCH()` 순환 수집 로직 없음 |
| **Fix Summary** | `searchedChannels` 추가; `seekCH()` 첫 반환값까지 반복 후 `std::sort` (`include/TVChannelController.h:92–105`) |

**관련 테스트:** `ChannelSearch_*` (6건), `ChannelSearchMockTest.*` (2건)

---

## DEF-002

| 필드 | 내용 |
|------|------|
| **ID** | DEF-002 |
| **Severity** | Critical |
| **ItemType** | Missing Implementation |
| **Status** | Fixed (`df4a393`) |
| **Steps** | 1. 채널 검색 없이 `tuner->setCH("6")`<br>2. `pressChannelUp()` 호출<br>3. `currentCh()` 확인 |
| **Expected** | 채널 **7** (`ChannelUp_NoSearch_From6_To7`, FR-05-01) |
| **Actual** | 채널 **6** 유지 (no-op 시) |
| **Root Cause** | `pressChannelUp()` 미구현. 검색 목록 유무 분기·`(ch+1)%100` 없음 |
| **Fix Summary** | `searchedChannels.empty()` 시 모듈로 100 순환 증가; 있으면 `std::upper_bound` 다음·wrap (`:107–121`) |

**관련 테스트:** `ChannelUp_NoSearch_*` (5건), `ChannelUp_WithSearch_*` (3건)

---

## DEF-003

| 필드 | 내용 |
|------|------|
| **ID** | DEF-003 |
| **Severity** | Critical |
| **ItemType** | Missing Implementation |
| **Status** | Fixed (`df4a393`) |
| **Steps** | 1. `FakeTuner({4,6,14})`로 검색 수행 후 CH=6 설정<br>2. `pressChannelDown()` 호출<br>3. `currentCh()` 확인 |
| **Expected** | 채널 **4** (`ChannelDown_WithSearch_OnList_6to4`, FR-06-01) |
| **Actual** | 채널 **6** 유지 |
| **Root Cause** | `pressChannelDown()` 미구현. 목록 내 이전·wrap·목록 외 최대값 분기 없음 |
| **Fix Summary** | `searchedChannels.empty()` 시 `(ch-1+100)%100`; 있으면 `lower_bound`/`upper_bound` 분기 (`:123–150`) |

**관련 테스트:** `ChannelDown_NoSearch_*` (5건), `ChannelDown_WithSearch_*` (5건)

---

## DEF-004

| 필드 | 내용 |
|------|------|
| **ID** | DEF-004 |
| **Severity** | Major |
| **ItemType** | Bug |
| **Status** | Fixed (`61c68e6`) |
| **Steps** | 1. 초기 CH=0<br>2. `pressNumber(4)`, `pressNumber(5)`, `pressNumber(6)` → CH **45**<br>3. `pressOther()` 호출<br>4. `currentCh()` 확인 |
| **Expected** | CH **45** 유지, 잔여 숫자 `6`만 버퍼 무효화 (`ThreeDigits_ThenOther_ClearsSix`, FR-01-04) |
| **Actual** | CH **0** (`EXPECT_EQ(45, currentCh())` 실패, `Which is: 0`) |
| **Root Cause** | `pressOther()`가 `buffer.clear()` 후 `tuner_.setCH("0")` 호출 (커밋 `815a16d`, `:54` 부근) |
| **Fix Summary** | `pressOther()`를 `buffer.clear()`만 수행하도록 변경; `setCH` 제거 |

---

## DEF-005

| 필드 | 내용 |
|------|------|
| **ID** | DEF-005 |
| **Severity** | Minor |
| **ItemType** | Bug |
| **Status** | Fixed (현재 HEAD Green) |
| **Steps** | 1. `pressNumber(4)`, `pressNumber(5)` → CH **45**<br>2. `pressNumber(6)` 후 `pressConfirm()`<br>3. `currentCh()` 확인 |
| **Expected** | CH **6** (`ThreeDigits_ApplyFirstTwoThenConfirm`, FR-01-04) |
| **Actual** | (과거) 3번째 숫자에서 `buffer.clear()`만 하고 확인 시 적용 불가 가능 |
| **Root Cause** | `pressNumber()` 3자리 분기(`buffer.size() >= 3` → clear)가 2자리 자동 `applyBuffer()`와 조합 시 버퍼 잔류 규칙 불명확 (`:40–49`) |
| **Fix Summary** | 2자리에서 `applyBuffer()` 후 3번째는 단일 자리 버퍼만 유지; `pressConfirm()`에서 `applyBuffer()` 적용 |

---

## DEF-006

| 필드 | 내용 |
|------|------|
| **ID** | DEF-006 |
| **Severity** | Minor |
| **ItemType** | Specification Gap |
| **Status** | Open |
| **Steps** | 1. `.cursorrules` — 3자리 이상 입력 `123` → CH **23** 규칙 확인<br>2. `TDD_TV_Requirements.txt` FR-01-04 — `456` → **45** 후 확인 시 **6** 규칙 확인<br>3. `pressNumber()` 구현 대조 |
| **Expected** | 단일 명세 (프로젝트 규칙 또는 README 중 하나) |
| **Actual** | README/요구사항(FR-01-04)과 `.cursorrules`(마지막 두 자리) 불일치. 현재 구현·테스트는 **FR-01-04** 기준 |
| **Root Cause** | 문서 3원 불일치 (README / `.cursorrules` / 구현). `Report/01`, `Report/02` §4.1 |
| **Fix Summary** | characterization test로 단일 규칙 고정 후 문서·구현 정렬; 변경 시 `pressNumber` 정책 클래스 분리 검토 |

---

## DEF-007

| 필드 | 내용 |
|------|------|
| **ID** | DEF-007 |
| **Severity** | Minor |
| **ItemType** | Specification Gap |
| **Status** | Open |
| **Steps** | 1. 채널 1, 7, 12 설정·조회<br>2. Tuner 문자열 표기 확인 |
| **Expected** | (`.cursorrules`) 두 자리 표기 `01`, `07`, `12` |
| **Actual** | `FakeTuner`·테스트는 `"1"`, `"7"`, `"12"` 등 가변 길이 문자열 사용 |
| **Root Cause** | 채널 표시 규칙 미통일 (`Report/01` §6) |
| **Fix Summary** | `Channel` 값 객체 또는 `to_string` 포맷 일원화; 테스트 `00`/`01`/`99` 케이스 추가 (Green 유지) |

---

## DEF-008

| 필드 | 내용 |
|------|------|
| **ID** | DEF-008 |
| **Severity** | Info |
| **ItemType** | Technical Debt |
| **Status** | Open |
| **Steps** | 1. `pressFavorite()` / `pressNextFavorite()` 반복 호출<br>2. 프로파일 또는 코드 리뷰 |
| **Expected** | 즐겨찾기 목록 조회 시 불필요 복사 최소화 |
| **Actual** | `getFavoriteChannels()`가 매 호출 `std::vector` 전체 복사 (`:71–73`) |
| **Root Cause** | API가 `set` → `vector` 변환 반환 (`Report/03` §5.2) |
| **Fix Summary** | `const std::set<int>&` 반환 또는 순회 API; 리팩토링 단계에서 적용 (기능 변경 없음) |

---

## DEF-009

| 필드 | 내용 |
|------|------|
| **ID** | DEF-009 |
| **Severity** | Minor |
| **ItemType** | Technical Debt |
| **Status** | Open |
| **Steps** | 1. 버퍼에 `9`,`9`,`1` 등 100 초과 조합 입력<br>2. `applyBuffer()` 동작 확인 |
| **Expected** | 0~99 범위 밖 채널은 거부 또는 클램프 (명세 확정 후) |
| **Actual** | `applyBuffer()`가 범위 검증 없이 `tuner_.setCH` 호출 (`:16–27`) |
| **Root Cause** | 방어적 검증 부재 (`Report/03` §4.4) |
| **Fix Summary** | `validateChannel(0..99)` 추가; 명세·테스트 선행 |

---

## DEF-010

| 필드 | 내용 |
|------|------|
| **ID** | DEF-010 |
| **Severity** | Minor |
| **ItemType** | Test Gap |
| **Status** | Open |
| **Steps** | 1. `pressNumber(-1)` 호출<br>2. `pressNumber(10)` 호출 |
| **Expected** | `std::invalid_argument` throw (E1, E2 — `Report/04` §5.1) |
| **Actual** | 구현은 throw (`:35–36`)하나 **단위 테스트 없음** |
| **Root Cause** | 예외 시나리오 `TEST_F` 미작성 (`Report/05` §8.1) |
| **Fix Summary** | `PressNumber_Negative_Throws`, `PressNumber_Ten_Throws` 추가 (`EXPECT_THROW`) |

---

## DEF-011

| 필드 | 내용 |
|------|------|
| **ID** | DEF-011 |
| **Severity** | Minor |
| **ItemType** | Test Gap |
| **Status** | Open |
| **Steps** | 1. `9,9,9` / `0,7,5` / `1,2,3,확인` 등 세 자리·4키 시퀀스 실행<br>2. `ctest` 실행 |
| **Expected** | B4~B7 경계 시나리오 통과 (`Report/04` §4) |
| **Actual** | 해당 `TEST_F` **미작성** (B1~B3만 존재) |
| **Root Cause** | 테스트 계획 P1 항목 미착수 |
| **Fix Summary** | `ThreeDigits_999`, `ThreeDigits_075` 등 `TEST_F` 추가 후 Green 유지 |

---

## DEF-012

| 필드 | 내용 |
|------|------|
| **ID** | DEF-012 |
| **Severity** | Info |
| **ItemType** | Test Gap |
| **Status** | Open |
| **Steps** | 1. `ENABLE_COVERAGE` 빌드<br>2. gcov/lcov 실행 |
| **Expected** | Line coverage ≥ 90% (DoD, `Report/04` §6) |
| **Actual** | 커버리지 **미측정** (`Report/05` §7 항목 4) |
| **Root Cause** | CI/로컬 lcov 절차 미실행 |
| **Fix Summary** | CMake coverage 옵션·lcov 스크립트 실행; 미달 시 분기·예외 테스트 보강 |

---

## DEF-013

| 필드 | 내용 |
|------|------|
| **ID** | DEF-013 |
| **Severity** | Info |
| **ItemType** | Technical Debt |
| **Status** | Open (Waived — 수정 금지) |
| **Steps** | 1. `TVController::pushButton(KEY_1)` / `KEY_OK` 호출<br>2. Tuner CH 변경 여부 확인 |
| **Expected** | (레거시) 리모컨 키 경로로 채널 변경 |
| **Actual** | `setTunerCh()` 내 `tuner->setCH` **주석 처리**; `processingCH`만 갱신 (`include/TVController.h:24–28`) |
| **Root Cause** | 레거시 `TVController` 미완성; 실제 기능은 `TVChannelController` 전용 API |
| **Fix Summary** | **TVController 수정 금지** — `TVChannelController` API만 사용·테스트. 통합 시 어댑터 레이어 별도 설계 |

---

## DEF-014

| 필드 | 내용 |
|------|------|
| **ID** | DEF-014 |
| **Severity** | Minor |
| **ItemType** | Test Gap |
| **Status** | Open |
| **Steps** | 1. 빈 숫자 버퍼 상태에서 `pressConfirm()` 호출 |
| **Expected** | no-op, Tuner CH 불변 (S2 — `Report/04` §5.2) |
| **Actual** | 동작은 no-op이나 **전용 테스트 없음** |
| **Root Cause** | P1 시나리오 미작성 |
| **Fix Summary** | `PressConfirm_EmptyBuffer_NoOp` `TEST_F` 추가 |

---

## 요약 매트릭스

| ID | Severity | ItemType | Status | 연관 FR | 실패 TEST_F (과거) |
|----|----------|----------|--------|---------|-------------------|
| DEF-001 | Critical | Missing Implementation | Fixed | FR-04 | 8건 |
| DEF-002 | Critical | Missing Implementation | Fixed | FR-05 | 10건 |
| DEF-003 | Critical | Missing Implementation | Fixed | FR-06 | 10건 |
| DEF-004 | Major | Bug | Fixed | FR-01-04 | 1건 |
| DEF-005 | Minor | Bug | Fixed | FR-01-04 | 1건 (잠재) |
| DEF-006 | Minor | Specification Gap | Open | FR-01 | — |
| DEF-007 | Minor | Specification Gap | Open | — | — |
| DEF-008 | Info | Technical Debt | Open | FR-02 | — |
| DEF-009 | Minor | Technical Debt | Open | FR-01 | — |
| DEF-010 | Minor | Test Gap | Open | — | — |
| DEF-011 | Minor | Test Gap | Open | FR-01 | — |
| DEF-012 | Info | Test Gap | Open | DoD | — |
| DEF-013 | Info | Technical Debt | Open (Waived) | — | — |
| DEF-014 | Minor | Test Gap | Open | FR-01 | — |

**ctest (HEAD):** 57 passed, 0 failed — DEF-001~005 수정 반영 완료.

---

## 참고 문서

| 문서 | 경로 |
|------|------|
| 결함 관리 보고서 | `Report/09_Defect_Management_Report.md` |
| 결함 분석 보고서 | `Report/06_TVChannelController_Defect_Analysis_Report.md` |
| 테스트 구현 보고서 | `Report/05_TVController_Test_Implementation_Report.md` |
| 테스트 계획 | `Report/04_Test_Plan_Report.md`, `docs/test_plan.md` |
| QA·리팩토링 | `Report/01_TVController_QA_Refactoring_Report.md` |
| 요구사항 | `TDD_TV_Requirements.txt` |
