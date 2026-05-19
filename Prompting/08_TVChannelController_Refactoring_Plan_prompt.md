# TVChannelController Refactoring Plan — Conversation Transcript
_Exported on 5/19/2026 from Cursor (current session)_

---

**User**

@include/TVChannelController.h @test/TVControllerTest.cpp @docs/code_quality_report.md @TDD_TV_Requirements.txt

[P] 모던 C++ 리팩토링 코치입니다.
[C] 제약: Tuner, TVController 구조체 수정 금지, channel 0~99, 테스트 Green에서만 진행
[T] TVChannelController 리팩토링 계획을 단계별로 제안해줘.
    - 조건 분기 축소/중복 제거 (커밋 단위로 쪼개기)
    - 타입/정책 분리 (전략 패턴/테이블 기반/함수 분해)
    - 매직 넘버 상수화와 C++17 스타일 개선
[F] 단계별 체크리스트 + 각 단계 검증 방법 (cmake --build && ctest)

---

**Cursor**

`TVChannelController.h`, `TVControllerTest.cpp`, `docs/code_quality_report.md`, `TDD_TV_Requirements.txt`를 읽고 Green 전제 하에 커밋 단위 리팩터링 계획을 제안했습니다.

### 핵심 요약

- **현재:** FR-01~06 구현 완료, 헤더 ~154줄, ctest 71건 Green 전제
- **목표 구조:** `Channel` + `DigitInputBuffer` + `FavoriteStore` + `ChannelSearchCollector` + `NavMode`(Wrap | SortedList)
- **진행:** Phase 0(기준선) → Commit 1~9(필수) → Commit 10(선택, .cpp 분리)
- **검증:** 매 커밋 `cmake --build build` + `ctest --test-dir build --output-on-failure`
- **주의:** FR-01-04는 2자리 자동 적용 후 버퍼 리셋 → 3번째 숫자는 새 1자리; Golden `fr01_*`로 고정

**커밋 요약**

| Commit | 내용 |
|--------|------|
| 1 | `Channel` + `ChannelLimits` 상수 |
| 2 | Tuner 게이트웨이 (`currentChannel` / `setChannel`) |
| 3 | `DigitInputBuffer` (FR-01) |
| 4 | `FavoriteStore` (FR-02) |
| 5 | `nextInSortedRing` 유틸 (FR-03/06) |
| 6 | `ChannelSearchCollector` (FR-04) |
| 7 | `NumericWrapNavigation` / `SortedListNavigation` |
| 8 | `pressChannelDown` 평탄화 |
| 9 | C++17 스타일 (`[[nodiscard]]` 등) |
| 10 | (선택) `.cpp` 분리 |

---

**User**

Report 디렉토리에 보고서 작성

---

**Cursor**

기존 Report 번호 체계에 맞춰 공식 보고서를 작성했습니다.

**파일:** `Report/08_TVChannelController_Refactoring_Plan_Report.md`

포함 내용:
- 목적·관련 산출물·현재 상태(154줄, ctest 71)
- 리팩터 원칙·목표 아키텍처(mermaid)
- Phase 0 + Commit 1~10 체크리스트·검증 테스트·커밋 메시지 예
- FR ↔ 컴포넌트 추적 매트릭스
- 공통 검증 절차·리스크·완료 정의

---

**User**

Prompting 디렉토리에 현재 프롬프트 추출해서 저장

---

**Cursor**

`Prompting/08_TVChannelController_Refactoring_Plan_prompt.md` (본 파일)에 현재 세션 대화 transcript를 export했습니다.

---

## 세션 산출물 요약

| 유형 | 경로 |
|------|------|
| 공식 보고서 | `Report/08_TVChannelController_Refactoring_Plan_Report.md` |
| 대화 transcript | `Prompting/08_TVChannelController_Refactoring_Plan_prompt.md` |
| 분석 근거 | `docs/code_quality_report.md`, `Report/03_TVChannelController_Code_Quality_Report.md` |

## 분석 대상 코드 스냅샷

- `include/TVChannelController.h` (~154줄, 헤더 인라인)
- `test/TVControllerTest.cpp` (44 `TEST_F`, FR-01~06)
- `test/TVControllerGoldenTest.cpp` (14 Golden 시나리오)
- 상속: `TVController` (레거시, 수정 금지)
- 구현 범위: FR-01~06 **구현 완료**, Green 기준 리팩터링만 진행

## 사용자 요청 프롬프트 템플릿 (재사용용)

```
@include/TVChannelController.h @test/TVControllerTest.cpp @docs/code_quality_report.md @TDD_TV_Requirements.txt

[P] 모던 C++ 리팩토링 코치입니다.
[C] 제약: Tuner, TVController 구조체 수정 금지, channel 0~99, 테스트 Green에서만 진행
[T] TVChannelController 리팩토링 계획을 단계별로 제안해줘.
    - 조건 분기 축소/중복 제거 (커밋 단위로 쪼개기)
    - 타입/정책 분리 (전략 패턴/테이블 기반/함수 분해)
    - 매직 넘버 상수화와 C++17 스타일 개선
[F] 단계별 체크리스트 + 각 단계 검증 방법 (cmake --build && ctest)
    → Report/08_TVChannelController_Refactoring_Plan_Report.md 로 저장
```

## 후속 작업 프롬프트 예 (Commit 1 실행)

```
@Report/08_TVChannelController_Refactoring_Plan_Report.md @include/TVChannelController.h

[P] 모던 C++ 리팩토링 코치입니다.
[C] Tuner/TVController/remoteKey 수정 금지. Report/08 Commit 1만 수행. 테스트 Green 유지.
[T] Channel 값 객체와 ChannelLimits 상수를 도입하고 TVChannelController 내부 stoi/to_string을 치환해줘.
[F] 변경 후 cmake --build build && ctest --test-dir build --output-on-failure 결과 보고
```

---

## Report/08 체크리스트 요약 (본문 전체는 Report/08 참조)

| Phase/Commit | 핵심 작업 |
|--------------|-----------|
| 0 | Green 기준선, (선택) `refactor-baseline` tag |
| 1 | `Channel`, `ChannelLimits` |
| 2 | `currentChannel` / `setChannel` 게이트웨이 |
| 3 | `DigitInputBuffer` + FR-01 정책 |
| 4 | `FavoriteStore::toggle` |
| 5 | `nextInSortedRing` |
| 6 | `ChannelSearchCollector` |
| 7 | `NavMode` variant (Wrap \| SortedList) |
| 8 | `pressChannelDown` 평탄화 |
| 9 | C++17 polish |
| 10 | (선택) `.cpp` 분리 |

---

*End of transcript*
