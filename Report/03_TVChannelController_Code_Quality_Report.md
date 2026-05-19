# 03. TVChannelController 코드 품질 분석 보고서

작성일: 2026-05-19

## 1. 목적

본 보고서는 `TVChannelController` 구조체(클래스)의 공개·비공개 멤버 함수를 **SOLID 원칙**과 **Code Smell** 관점에서 분석하고, **C++17** 기반 리팩토링 방향 및 우선순위를 제시한다.

| 구분 | 설명 |
|------|------|
| 대상 | `include/TVChannelController.h` |
| 관점 | SRP/OCP, Long Method, Duplicated Code, 조건문 복잡도 |
| 기준 문서 | `TDD_TV_Requirements.txt`, `README.md` |
| 선행 보고서 | `Report/01_TVController_QA_Refactoring_Report.md`, `Report/02_Requirements_Analysis_Report.md` |

**관련 산출물**

| 문서 | 경로 | 역할 |
|------|------|------|
| 본 보고서 | `Report/03_TVChannelController_Code_Quality_Report.md` | SOLID/스멜 분석, 리팩토링 우선순위 |
| 동일 내용 사본 | `docs/code_quality_report.md` | 개발 참고용 요약 |

---

## 2. 구조 개요

`TVChannelController`는 단일 헤더(~90줄)에 **숫자 입력 버퍼**, **Tuner 채널 적용**, **즐겨찾기 CRUD/토글**, **다음 즐겨찾기 탐색**을 모두 담고 있다. `TVController`를 상속하지만 테스트·확장 경로는 `pressNumber` 등 전용 API에 의존하며, 부모의 `pushButton(remoteKey)` 경로와 **이중 튜너 참조**(`TVController::tuner*`, `tuner_`)가 공존한다.

향후 FR-04(채널 검색), FR-05/06(업·다운)까지 같은 클래스에 누적될 경우 **God Class**로 성장할 위험이 크다.

---

## 3. SOLID / Code Smell 분석 표

| 문제점 | 위반 원칙 / 스멜 | 영향 | 개선 방향 | 우선순위 |
|--------|------------------|------|-----------|----------|
| 한 클래스가 숫자 입력·버퍼·튜너 반영·즐겨찾기 저장·다음 즐겨찾기 탐색을 모두 담당 | **SRP** 위반, **God Class** 징후 | FR-04~06 추가 시 변경 범위·회귀 위험 증가, 단위 테스트 격리 어려움 | `DigitInputBuffer`, `FavoriteStore`, `ChannelNavigator`(다음 즐겨찾기)로 역할 분리; Controller는 조합만 담당 | **1** |
| `pressNumber` 내부에 2자리 자동 적용·3자리 무효화 등 **입력 정책이 하드코딩** | **OCP** 위반, **Switch/조건 분기 확장** 스멜 | README/`.cursorrules`의 3자리 규칙 변경·FR 연동 시 메서드 전체 수정 | `IDigitBufferPolicy` 또는 **전이 테이블**(상태×이벤트→동작); 정책 교체는 테스트로 고정 | **2** |
| FR-04~06(검색·업/다운)을 동일 클래스에 추가할 구조 없음 | **OCP** 위반 (미래) | 기능마다 `if (hasSearchResult)` 분기 증가, **Long Method**·**높은 순환 복잡도** | `std::variant` 또는 **커맨드/핸들러 테이블**; 검색 유무는 `INavigationStrategy` 주입 | **3** |
| `std::stoi(tuner_.getCurrentCH())`가 `pressFavorite`, `pressNextFavorite`에 중복 | **Duplicated Code**, **Feature Envy** | 파싱 실패·`"07"` vs `"7"` 불일치 시 버그, 수정 시 다중 위치 변경 | `Channel` 값 객체; `getCurrentChannel()` 한 곳에서만 `stoi` | **1** |
| `tuner_.setCH(std::to_string(...))` 반복 | **Duplicated Code** | 00~99 포맷 규칙 변경 시 누락 | `Channel::applyTo(Tuner&)` 단일 진입점 | **1** |
| `pressNumber` 중첩 `if` + 매직 넘버 (2, 3, 9) | **조건문 복잡도** | 분기 폭증; FR-01-04(3번째 숫자 잔류)와 `buffer.clear()` 불일치 | `kMaxBufferDigits` 등 명명 상수; 상태 기계·테이블 | **2** |
| `TVController` 상속 + 미사용 `pushButton` / 이중 `Tuner` | **SRP**, **Dead Code** | 혼란, 잘못된 튜너 참조 가능성 | 채널 변경은 `tuner_` 단일 경로; 레거시는 생성 위임만 | **4** |
| `applyBuffer` 범위 검증 없음 | **방어적 설계 부재** | 100 이상 조합 시 Tuner 예외 의존 | `validateChannel(0..99)` (명세 확정 후) | **2** |
| `getFavoriteChannels()` 매 호출 `vector` 복사 | **성능 스멜** (경미) | 불필요 할당 | `const std::set<int>&` 또는 순회 API | **5** |
| 구현 전체 헤더 인라인 | **컴파일 의존성** | 변경 시 전체 재빌드 | `.cpp` 분리 (Green 후) | **5** |
| `pressFavorite` find/insert·erase 분기 | **중복 패턴** | 가독성 저하 | `FavoriteStore::toggle(int)` | **3** |
| `pressNextFavorite` wrap-around 분기 | **조건문 복잡도** | FR-06 업/다운과 로직 중복 예상 | `nextInSorted(set, current)` 공용 유틸 | **3** |

---

## 4. SRP / OCP 위반 — 근거

### 4.1 SRP (Single Responsibility Principle)

| 책임 | 현재 위치 | 이상적 소유 |
|------|-----------|-------------|
| 숫자 버퍼 누적·해석·적용 시점 | `buffer`, `applyBuffer`, `pressNumber` | `DigitInputBuffer` |
| Tuner 채널 읽기/쓰기 | 각 `press*` 메서드 | `TunerChannelGateway` / `Channel` |
| 즐겨찾기 집합 관리 | `favorites`, `pressFavorite`, `addFavorite` | `FavoriteStore` |
| 정렬 집합에서 “다음” 탐색 | `pressNextFavorite` | `FavoriteNavigator` / `SortedRingNavigator` |

**근거:** 변경 이유가 입력 규칙·즐겨찾기 정책·탐색 알고리즘으로 나뉜다. FR-01~06은 독립 시나리오이므로 **변경 축 단위 분리**가 SRP에 부합한다.

### 4.2 OCP (Open-Closed Principle)

- **현재:** FR-04~06 추가 시 `TVChannelController`에 메서드·멤버·`if`를 직접 추가·수정해야 한다.
- **개선:** `INavigationStrategy`(`NumericWrap`, `SearchedList`, `FavoriteNext`), 상위 `remoteKey`→handler 테이블(레거시 enum 미수정), `std::variant` 기반 모드 분리.

**근거:** 요구사항 §3.4~3.6은 검색 결과 유무에 따라 업/다운 의미가 **완전히 달라진다**. 단일 `pressChannelUp()`에 누적하면 OCP·복잡도가 동시에 악화된다.

---

## 5. Code Smell 상세

### 5.1 Long Method

`pressNumber`(17줄), `pressNextFavorite`(14줄)은 현재는 짧으나, FR-04~06 구현 시 검색 루프·목록 저장·업/다운이 합쳐지면 **40줄+ Long Method** 및 **Brain Class** 위험이 있다.

### 5.2 Duplicated Code

```cpp
int current = std::stoi(tuner_.getCurrentCH());  // pressFavorite, pressNextFavorite
tuner_.setCH(std::to_string(value));             // applyBuffer, pressNextFavorite
```

자릿수 합산 루프는 `Channel::fromDigits` 등으로 FR-01 전용 로직과 분리 가능하다.

### 5.3 조건문 복잡도

`pressNumber` 의사결정: `valid? → push → size>=3? clear : size==2? apply`

README FR-01-04(45 유지 + `6` 잔류)와 구현(`buffer.clear()`)은 **명세 부채**이기도 하다. 리팩터 전 TS-106 등 characterization test로 단일 진실 공급원을 고정할 것.

---

## 6. C++17 개선 방향

| 접근 | 용도 | 비고 |
|------|------|------|
| **값 객체 `Channel`** | `stoi`/`to_string`·0~99 검증 일원화 | 우선순위 1 |
| **전략 `INavigationStrategy`** | FR-03/05/06 탐색·업다운 | 검색 완료 시 strategy 교체 |
| **테이블 기반 버퍼** | FR-01 digit/confirm 이벤트 | `BufferAction` + `constexpr` lookup |
| **`std::variant` NavContext** | 검색 있음/없음/즐겨찾기 모드 | 소규모 실습은 전략 2~3개가 단순할 수 있음 |
| **`std::optional<Channel>`** | Tuner 파싱 실패 처리 | C++17 |
| **명명 상수** | `kMinChannel`, `kMaxChannel`, `kMaxDigit` | 매직 넘버 제거 |

---

## 7. 리팩토링 우선순위 (1~5)

| 순위 | 작업 | 이유 |
|------|------|------|
| **1** | `Channel` + Tuner 읽기/쓰기 단일화 | 변경 면적 작음, 모든 FR의 공통 기반 |
| **2** | `DigitInputBuffer` 추출 + 3자리 명세 테스트(TS-106) | FR-01·QA 이슈; SRP/OCP 동시 개선 |
| **3** | `FavoriteStore` / `nextInSorted` 유틸 | FR-02/03 안정, FR-06 재사용 |
| **4** | `INavigationStrategy` 후 FR-04~06 구현 | 분기 폭증 방지 |
| **5** | 헤더 분리, `getFavoriteChannels` 최적화 | 기능 완성 후 정리 |

**제약:** `TDD_TV_Requirements.txt` §4.3 — 테스트 Green, 동작 변경과 구조 변경 분리, `TVController`/`Tuner`/`remoteKey` 미변경.

---

## 8. 개선 방향 요약

1. **채널 표현 일원화** — Tuner 문자열↔정수·0~99 검증을 `Channel`에 모은다.
2. **입력 버퍼 분리** — `pressNumber` 분기를 상태/테이블로 옮기고, FR-01-04는 테스트 선행으로 명세를 고정한다.
3. **탐색·업다운은 전략** — FR-04~06을 `if` 누적 대신 `INavigationStrategy`로 확장한다(OCP).
4. **즐겨찾기 저장/탐색 분리** — `toggle`은 Store, `pressNextFavorite`는 Navigator; FR-06 순환과 공통화한다.
5. **레거시 상속 최소화** — 신규 기능은 composition·명시 API로만 확장한다.

---

## 9. 참고 코드 위치

| 구간 | 파일 | 라인(대략) |
|------|------|------------|
| 버퍼 적용 | `include/TVChannelController.h` | 15–27 |
| 숫자 입력 분기 | `include/TVChannelController.h` | 33–48 |
| 즐겨찾기 토글 | `include/TVChannelController.h` | 56–66 |
| 다음 즐겨찾기 | `include/TVChannelController.h` | 75–88 |

---

## 10. 검증

리팩터링 실행 전·후 다음을 확인한다.

```powershell
ctest --test-dir "build" --output-on-failure
```

현재 기준: 24개 테스트 Green (`Report/01` 참조).

---

*정적 분석 기준 문서. 구현 변경은 TDD Red→Green→Refactor 순서를 따른다.*
