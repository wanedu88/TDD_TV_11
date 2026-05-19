# TVChannelController 코드 품질 분석 (요약)

> **공식 보고서:** [`Report/03_TVChannelController_Code_Quality_Report.md`](../Report/03_TVChannelController_Code_Quality_Report.md)

| 항목 | 내용 |
|------|------|
| 대상 | `include/TVChannelController.h` |
| 관점 | SOLID, Code Smell, C++17 모던화 |
| 기준 문서 | `TDD_TV_Requirements.txt`, `README.md` |
| 작성일 | 2026-05-19 |
---

## 1. 구조 개요

`TVChannelController`는 단일 헤더(~90줄)에 **숫자 입력 버퍼**, **Tuner 채널 적용**, **즐겨찾기 CRUD/토글**, **다음 즐겨찾기 탐색**을 모두 담고 있다. `TVController`를 상속하지만 테스트·확장 경로는 `pressNumber` 등 전용 API에 의존하며, 부모의 `pushButton(remoteKey)` 경로와 **이중 튜너 참조**(`TVController::tuner*`, `tuner_`)가 공존한다.

향후 FR-04(채널 검색), FR-05/06(업·다운)까지 같은 클래스에 누적될 경우 **God Class**로 성장할 위험이 크다.

---

## 2. SOLID / Code Smell 분석 표

| 문제점 | 위반 원칙 / 스멜 | 영향 | 개선 방향 | 우선순위 |
|--------|------------------|------|-----------|----------|
| 한 클래스가 숫자 입력·버퍼·튜너 반영·즐겨찾기 저장·다음 즐겨찾기 탐색을 모두 담당 | **SRP** 위반, **God Class** 징후 | FR-04~06 추가 시 변경 범위·회귀 위험 증가, 단위 테스트 격리 어려움 | `DigitInputBuffer`, `FavoriteStore`, `ChannelNavigator`(다음 즐겨찾기)로 역할 분리; Controller는 조합만 담당 | **1** |
| `pressNumber` 내부에 2자리 자동 적용·3자리 무효화 등 **입력 정책이 하드코딩** | **OCP** 위반, **Switch/조건 분기 확장** 스멜 | README/`.cursorrules`의 3자리 규칙 변경·FR 연동 시 메서드 전체 수정 | `IDigitBufferPolicy` 또는 **전이 테이블**(상태×이벤트→동작); 정책 교체는 테스트로 고정 | **2** |
| FR-04~06(검색·업/다운)을 동일 클래스에 추가할 구조 없음 | **OCP** 위반 (미래) | 기능마다 `if (hasSearchResult)` 분기 증가, **Long Method**·**높은 순환 복잡도** | `std::variant<Idle, Buffering, …>` 또는 **커맨드/핸들러 테이블** (`pressX` → `Handler`); 검색 유무는 `INavigationStrategy` 주입 | **3** |
| `std::stoi(tuner_.getCurrentCH())`가 `pressFavorite`, `pressNextFavorite`에 중복 | **Duplicated Code**, **Feature Envy** (Tuner 문자열 형식에 의존) | 파싱 실패 시 예외 전파, `"07"` vs `"7"` 불일치 시 버그, 수정 시 다중 위치 변경 | `Channel` 값 객체(`int` + `toTunerString()`/`fromTuner`); `getCurrentChannel()` 한 곳에서만 `stoi` | **1** (SRP와 동시 진행) |
| `tuner_.setCH(std::to_string(...))` 반복 (`applyBuffer`, `pressNextFavorite`) | **Duplicated Code** | 채널 00~99 포맷 규칙 변경 시 누락 | `Channel::applyTo(Tuner&)` 단일 진입점; 필요 시 `std::format`/`snprintf`로 2자리 고정 | **1** |
| `pressNumber`의 중첩 `if` (유효성 → push → size≥3 → size==2) | **조건문 복잡도**, **매직 넘버** (2, 3, 9) | 규칙 추가 시 분기 폭증; FR-01-04(3번째 숫자 잔류)와 현재 `clear` 동작 불일치 | 명명 상수 `kMaxBufferDigits`, `kAutoApplyDigits`; 상태 기계 또는 테이블로 분기 축소 | **2** |
| `TVController` 상속 + 미사용 `pushButton` / 이중 `Tuner` 참조 | **SRP** (책임 혼재), **Dead Code** | 혼란, 잘못된 튜너 인스턴스 사용 가능성(레거시 수정 불가로 완전 제거는 어려움) | 확장 API만 문서화; 가능하면 `TVController`는 생성자 위임만, **모든 채널 변경은 `tuner_` 단일 경로**로 통일 | **4** |
| `applyBuffer`가 범위 검증 없이 `stoi` 조합값을 Tuner에 전달 | **방어적 설계 부재** | 100 이상 조합·비정상 버퍼 시 Tuner 예외에 의존 | `validateChannel(0..99)`; 실패 시 버퍼만 clear 또는 no-op (요구 명세 확정 후) | **2** |
| `getFavoriteChannels()`가 매 호출 `vector` 복사 | **성능 스멜** (경미) | 테스트·UI 빈번 조회 시 불필요 할당 | `const std::set<int>&` 반환 또는 `span`/콜백 순회 API | **5** |
| 구현 전체가 헤더 인라인 | **컴파일 의존성** 스멜 | 변경 시 전체 테스트 타겟 재빌드 | `.cpp` 분리 또는 pimpl (리팩터 Green 후) | **5** |
| `pressFavorite`의 find/insert vs erase 분기 | **중복 패턴** (다른 CRUD에도 반복 가능) | 가독성 저하 | `FavoriteStore::toggle(int)`; C++17 `std::set::insert` 반환값 활용 | **3** |
| `pressNextFavorite` wrap-around 분기 | **조건문 복잡도** (낮음) | FR-06 목록 내 업/다운과 로직 중복 예상 | `nextInSorted(set, current)` 유틸 공유; 업/다운은 방향 파라미터 | **3** |

---

## 3. SRP / OCP 위반 — 근거 요약

### 3.1 SRP (Single Responsibility Principle)

| 책임 | 현재 위치 | 이상적 소유 |
|------|-----------|-------------|
| 숫자 버퍼 누적·해석·적용 시점 | `buffer`, `applyBuffer`, `pressNumber` | `DigitInputBuffer` |
| Tuner 채널 읽기/쓰기 어댑터 | 각 `press*` 메서드 내부 | `TunerChannelGateway` / `Channel` |
| 즐겨찾기 집합 관리 | `favorites`, `pressFavorite`, `addFavorite` | `FavoriteStore` |
| 정렬 집합에서 “다음” 탐색 | `pressNextFavorite` | `FavoriteNavigator` 또는 공용 `SortedRingNavigator` |

**근거:** 클래스 변경 이유가 “입력 규칙 변경”, “즐겨찾기 정책 변경”, “탐색 알고리즘 변경”으로 여러 개다. TDD 요구(FR-01~06)는 각각 독립 시나리오이므로 **변경 축(axis) 단위로 클래스를 나누는 것이 SRP에 부합**한다.

### 3.2 OCP (Open-Closed Principle)

- **닫혀 있지 않음:** 새 리모컨 동작(FR-04 검색, FR-05/06 업·다운)은 `TVChannelController`에 메서드·멤버·`if`를 **추가·수정**해야 한다.
- **열 수 있는 방향:**
  - **전략:** `INavigationStrategy` — `NumericWrapStrategy`(0~99), `SearchedListStrategy`, `FavoriteNextStrategy`.
  - **테이블:** `remoteKey` → handler 매핑(레거시 `remoteKey` 수정 불가 시, 상위 어댑터 레이어에서 `unordered_map<remoteKey, std::function<void()>>`).
  - **variant 상태:** `using ControllerMode = std::variant<BufferingState, FavoriteEditingState, …>;` — 모드별로 허용 이벤트만 처리.

**근거:** 요구사항 문서 §3.4~3.6은 “검색 결과 유무”에 따라 업/다운 의미가 **완전히 바뀐다**. 이를 하나의 `pressChannelUp()` 안에 쌓으면 OCP·복잡도 모두 악화된다.

---

## 4. Code Smell 상세

### 4.1 Long Method

현재 `pressNumber`(17줄), `pressNextFavorite`(14줄)은 아직 “Long” 수준은 아니나, FR-04~06 구현 시 **한 메서드 40줄+** 가능성이 높다. 특히 검색 루프 + 목록 저장 + 업/다운이 합쳐지면 **Long Method + Brain Class**로 이어진다.

### 4.2 Duplicated Code

```cpp
// 패턴 A — 현재 채널 int 변환 (2곳)
int current = std::stoi(tuner_.getCurrentCH());

// 패턴 B — 채널 설정 (2곳 이상)
tuner_.setCH(std::to_string(value));
```

`applyBuffer`의 자릿수 합산 루프는 FR-01 전용이지만, **채널 번호 합성**은 `Channel::fromDigits(span<const int>)`로 재사용 가능하다.

### 4.3 조건문 복잡도

`pressNumber` 의사결정 트리:

```
valid? → push → size>=3? → clear return
              → size==2? → applyBuffer
```

3자리 처리가 README(FR-01-04: 45 유지 + `6` 잔류)와 구현(`buffer.clear()`)에서 **분기 의미가 다름** — 조건 복잡도 문제이기도 하고 **명세 부채**이기도 하다. 리팩터 전 TS-106 등 **characterization test**로 단일 진실 공급원을 고정해야 한다.

---

## 5. C++17 개선 방향

### 5.1 값 객체 + 게이트웨이 (우선 적용)

```cpp
struct Channel {
  int value;
  static Channel fromTuner(const std::string& s);
  std::string toTunerString() const;  // 00~99 정책 한곳
  static bool isValid(int ch) noexcept;
};
```

### 5.2 전략 패턴 (FR-05 / FR-06 / FR-03)

```cpp
struct INavigationStrategy {
  virtual ~INavigationStrategy() = default;
  virtual Channel next(Channel current) const = 0;
  virtual Channel prev(Channel current) const = 0;
};
// NumericWrapNavigation | SearchedChannelListNavigation | FavoriteNextNavigation
```

Controller는 `std::unique_ptr<INavigationStrategy>` 또는 `std::variant` of strategies를 보유하고, 검색 완료 시에만 strategy를 교체(OCP).

### 5.3 테이블 기반 숫자 버퍼 (FR-01)

| 버퍼 길이 (입력 후) | 이벤트 | 동작 |
|---------------------|--------|------|
| 1 | Confirm | apply 1자리 |
| 2 | Digit | apply 2자리, clear |
| 3 | Digit | 정책 A: clear / 정책 B: 마지막 2자리 적용 (명세 확정 후) |

`enum class BufferAction { None, Apply, Clear, ApplyLastTwo };` + `constexpr` lookup table.

### 5.4 `std::variant` 가능성

```cpp
using NavContext = std::variant<
    std::monostate,              // 기본 0~99
    std::vector<Channel>,        // FR-04 검색 결과
    std::reference_wrapper<const std::set<Channel>>  // favorites
>;
```

- **장점:** “검색 있음/없음”을 타입으로 표현, 잘못된 상태 조합 컴파일 타임 감소.
- **단점:** 방문자 보일러플레이트; 소규모 실습에서는 **전략 인터페이스 2~3개**가 더 읽기 쉬울 수 있음.

### 5.5 기타 C++17

- `constexpr int kMinChannel = 0, kMaxChannel = 99, kMaxDigit = 9;`
- `if (auto ch = tryParseCurrent())` 스타일 — `std::optional<Channel>`
- `string_view`는 Tuner API가 `std::string` 고정이라 **경계에서만** 사용

---

## 6. 리팩토링 우선순위 (1~5)

| 순위 | 작업 | 이유 |
|------|------|------|
| **1** | `Channel` + Tuner 읽기/쓰기 단일화 (`stoi`/`to_string` 제거) | 변경 면적 작고, 모든 FR의 공통 기반; 회귀 테스트로 안전하게 적용 가능 |
| **2** | `DigitInputBuffer` 추출 + 명세(3자리) 테스트 고정(TS-106) | FR-01 핵심·현재 QA 이슈; SRP·OCP(입력 정책) 동시 개선 |
| **3** | `FavoriteStore` / `nextInSorted` 유틸 분리 | FR-02/03 안정; FR-06과 탐색 로직 재사용 |
| **4** | `INavigationStrategy` 도입 후 FR-04~06 **Green 단계에서** 구현 | OCP 확보; 검색·업다운 추가 전 구조를 잡아 분기 폭증 방지 |
| **5** | 헤더/구현 분리, `getFavoriteChannels` 반환 최적화 | 동작 불변·가독성·빌드 시간; 기능 완성 후 정리 단계 |

**원칙:** `TDD_TV_Requirements.txt` §4.3 — 테스트 Green 유지, 동작 변경과 구조 변경 분리, 작은 커밋 단위.

---

## 7. 개선 방향 요약

1. **채널 표현을 한곳으로** — Tuner 문자열↔정수 변환과 0~99 검증을 `Channel`에 모아 Duplicated Code·Feature Envy를 제거한다.
2. **입력 버퍼를 독립 컴포넌트로** — `pressNumber`의 조건 분기를 상태/테이블로 옮기고, FR-01-04 명세 불일치는 **테스트 먼저**로 해결한다.
3. **탐색·업다운은 전략으로 확장** — FR-04~06을 `TVChannelController`에 `if`로 쌓지 않고, 검색 결과·즐겨찾기·숫자 래핑을 교체 가능한 `INavigationStrategy`로 분리한다(OCP).
4. **즐겨찾기는 저장과 탐색 분리** — `toggle`/`add`는 Store, `pressNextFavorite`는 Navigator; FR-06 목록 순환과 공통화한다.
5. **레거시 상속은 최소 활용** — `TVController`/`remoteKey` 수정 불가 제약 하에, 신규 기능은 조합(composition)과 명시 API로만 확장한다.

위 순서대로 진행하면, 현재 ~90줄 헤더를 유지하면서도 FR-04~06 추가 시 **God Class·Long Method·분기 폭증**을 피할 수 있다.

---

## 8. 참고 코드 위치

| 구간 | 파일 | 라인(대략) |
|------|------|------------|
| 버퍼 적용 | `TVChannelController.h` | 15–27 |
| 숫자 입력 분기 | `TVChannelController.h` | 33–48 |
| 즐겨찾기 토글 | `TVChannelController.h` | 56–66 |
| 다음 즐겨찾기 | `TVChannelController.h` | 75–88 |

---

*본 문서는 정적 분석 기준이며, 리팩터링 실행 전 `ctest --test-dir build`로 Green 상태를 확인할 것.*
