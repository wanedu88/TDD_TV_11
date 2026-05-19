# TDD_TV 요구사항 분석 (C++ 구현 관점)

| 항목 | 내용 |
|------|------|
| **역할** | 시니어 C++ QA 엔지니어 관점 요구사항 재정리 |
| **대상** | TDD_TV C++17 (CMake, Google Test / Google Mock) |
| **근거** | `README.md`, `TDD_TV_Requirements.txt` |
| **범위** | `TVChannelController` 및 Tuner 연동 로직 (레거시 `TVController`, `Tuner`, `remoteKey` 미수정) |

---

## 0. 구현 아키텍처 요약

| 계층 | 타입 | 책임 |
|------|------|------|
| 입력 | `remoteKey` / 테스트 헬퍼 (`pressNumber`, `pressConfirm` 등) | 리모컨 이벤트 → Controller 메서드 |
| 도메인 | `TVChannelController` | 숫자 버퍼, 선호 채널, 검색 목록, 업/다운 규칙 |
| 하드웨어 추상화 | `Tuner` (`std::string` 채널) | `setCH`, `getCurrentCH`, `seekCH` |
| 테스트 대역 | `FakeTuner` / `MockTuner` | 결정적 채널·검색 동작 |

**채널 표현:** Tuner API는 `std::string`이나, 도메인 연산(비교·순환·선호 목록)은 **`int` 0~99** 로 정규화한 뒤 `std::to_string`으로 `setCH` 호출하는 것이 안전하다.

---

## 1. 기능별 규칙 표

### 1.1 채널 변경 (숫자 버튼 + 확인)

| 규칙 ID | 입력 시퀀스 | 버퍼/상태 | `Tuner::setCH` 호출 시점 | 기대 채널 (int) | 비고 |
|---------|-------------|-----------|--------------------------|-----------------|------|
| CH-01 | `1` → 확인 | 버퍼 `[1]` → 적용 후 clear | 확인 시 | 1 | 한 자리는 확인 필수 |
| CH-02 | `1` → `2` | 2자리 도달 시 즉시 적용 | 2번째 숫자 입력 시 | 12 | 확인 불필요 |
| CH-03 | `1`,`2`,`3`,`4` | `12` 적용 후 버퍼 `[3,4]` → `34` | 2자리마다 자동 | 최종 34 | 연속 2자리 × N회 |
| CH-04a | `4`,`5`,`6` | `45` 적용; 3번째 `6`은 **README: 잔여 digit** / **현 구현: 버퍼 clear** | 2자리 시 + (후속 확인 시) | 45 → (확인+`6` 시 6) | **명세·구현 불일치 — §3.3 참고** |
| CH-04b | CH-04a 이후 `6` + 확인 | 잔여 digit `[6]` 적용 | 확인 시 | 6 | README 명시 |
| CH-04c | CH-04a 이후 **그 외** 버튼 | 잔여 `6` 무효화 | 없음 (또는 버퍼만 clear) | 45 유지 | `pressOther()` 등 |
| CH-05 | `0` → `7` | 2자리 `07` → int 7 | 2번째 숫자 시 | 7 | 선행 `0`은 값에 반영되나 결과는 7 |

**구현 스케치 (C++):**

```cpp
// 버퍼: std::vector<int> digits;  // 0~9
// 적용: int ch = accumulate(digits);  // value = value*10 + d
// tuner_.setCH(std::to_string(ch));
// 2자리 완성 시 applyBuffer(); 1자리는 pressConfirm() 시 applyBuffer()
```

| 상수 (권장) | 값 |
|-------------|-----|
| `kMinChannel` | 0 |
| `kMaxChannel` | 99 |
| `kAutoApplyDigitCount` | 2 |
| `kMaxBufferBeforeThirdDigit` | 2 (README 3자리 시나리오 기준) |

---

### 1.2 선호 채널 추가

| 규칙 ID | 트리거 | 전제 | 동작 | 자료구조 | `Tuner` 호출 |
|---------|--------|------|------|----------|--------------|
| FAV-01 | 선호채널추가 | 현재 채널 ∉ favorites | `favorites.insert(current)` | `std::set<int>` 권장 | 없음 |
| FAV-02 | 선호채널추가 | 현재 채널 ∈ favorites | `favorites.erase(current)` | 동일 | 없음 |

| 구현 포인트 | 설명 |
|-------------|------|
| 현재 채널 읽기 | `int cur = std::stoi(tuner_.getCurrentCH());` |
| 중복 방지 | `set`/`unordered_set` + insert/erase |
| 채널 변경 없음 | 선호 토글만; `setCH` 호출하지 않음 |

---

### 1.3 다음 선호 채널

| 규칙 ID | 트리거 | 전제 | 동작 | `setCH` 인자 |
|---------|--------|------|------|--------------|
| NFAV-01 | 다음선호채널 | favorites 비어 있지 않음, `cur < max(favorites)`에 해당하는 다음 존재 | `upper_bound(cur)` → 최소 초과값 | `std::to_string(*it)` |
| NFAV-02 | 다음선호채널 | `cur`보다 큰 선호 없음 | wrap → `*favorites.begin()` (최소값) | 동일 |
| NFAV-03 | 다음선호채널 | favorites.empty() | no-op | — |

**예시 (int 비교):**

| favorites (정렬) | current | 결과 |
|------------------|---------|------|
| {1, 4, 12, 56} | 6 | 12 (`upper_bound(6)` → 12) |
| {1, 4, 12, 56} | 56 | 1 (wrap) |

```cpp
auto it = favorites.upper_bound(current);
int next = (it == favorites.end()) ? *favorites.begin() : *it;
tuner_.setCH(std::to_string(next));
```

---

### 1.4 채널 검색

| 규칙 ID | 트리거 | 전제 | 동작 | 저장 구조 |
|---------|--------|------|------|-----------|
| SRCH-01 | 채널검색 | — | `seekCH()` 반복 호출, 시청 가능 채널 수집 | `std::vector<int>` 또는 `std::set<int>` (중복 제거) |
| SRCH-02 | (암묵) | `seekCH()` 빈 문자열 또는 종료 조건 | 루프 종료 | TunerTest: 10회 반복 패턴 참고 |

| 구현 포인트 | 설명 |
|-------------|------|
| Tuner 부담 | 검색·채널 변경은 Tuner 내부; Controller는 **결과 목록만** 보관 |
| Mock 검증 | `EXPECT_CALL(tuner, seekCH()).Times(N)` |
| Fake | `FakeTuner::available_` 순회와 동일 패턴 가능 |

**미구현 상태:** README FR-04 — Controller에 검색 목록 멤버·`pressChannelSearch()` 등 추가 필요.

---

### 1.5 채널 업 (Channel Up)

#### A. 검색 결과 **없음** (`searchedChannels_.empty()`)

| 규칙 ID | current | 동작 | 결과 |
|---------|---------|------|------|
| UP-A01 | 6 | `cur + 1` (mod 100) | 7 |
| UP-A02 | 99 | wrap | 0 |

```cpp
int next = (current == 99) ? 0 : current + 1;
```

#### B. 검색 결과 **있음** (`searchedChannels_` 정렬됨)

| 규칙 ID | current | 동작 | 결과 |
|---------|---------|------|------|
| UP-B01 | 6, 목록 {4,6,14} | 목록에서 **엄격히 큰 값** 중 최소 | 14 |
| UP-B02 | 15, 목록 {4,6,14} | 현재가 목록 밖 → **순환** (다음으로 최소값) | 4 |

```cpp
// sorted list: channels
auto it = std::upper_bound(channels.begin(), channels.end(), current);
int next = (it == channels.end()) ? channels.front() : *it;
```

---

### 1.6 채널 다운 (Channel Down)

#### A. 검색 결과 **없음**

| 규칙 ID | current | 동작 | 결과 |
|---------|---------|------|------|
| DN-A01 | 6 | `cur - 1` (mod 100) | 5 |
| DN-A02 | 0 | wrap | 99 |

```cpp
int next = (current == 0) ? 99 : current - 1;
```

#### B. 검색 결과 **있음**

| 규칙 ID | current | 동작 | 결과 |
|---------|---------|------|------|
| DN-B01 | 6, 목록 {4,6,14} | 목록에서 **엄격히 작은 값** 중 최대 | 4 |
| DN-B02 | 15, 목록 {4,6,14} | 현재가 목록 밖 → **순환** (이전으로 최대값) | 14 |

```cpp
auto it = std::lower_bound(channels.begin(), channels.end(), current);
int prev = (it == channels.begin()) ? channels.back() : *(--it);
// current가 목록에 없을 때: README 예시는 wrap to max below or first in cycle — 명세대로 characterization test 권장
```

> **QA 주의:** 업/다운 “목록 밖 현재 채널” 시 wrap 규칙은 `upper_bound`/`lower_bound` 조합으로 **테스트 먼저 고정**할 것 (FR-06-02).

---

## 2. 문자열 비교·분기 시 주의점

Tuner 계약상 채널은 `std::string`이지만, **비즈니스 규칙은 정수 채널**이다. 문자열 API를 그대로 비교·분기하면 결함이 난다.

### 2.1 피해야 할 패턴

| 안티패턴 | 문제 | 예 |
|----------|------|-----|
| `getCurrentCH() == "7"` | `"07"`, `"7"` 불일치 | 동일 채널인데 실패 |
| `ch1 < ch2` (lexicographic) | `"9" > "12"` (문자열) | 업/다운·다음 선호 오류 |
| `ch.find("1")` | 부분 문자열 오탐 | `"12".find("1")` 성공 |
| `EXPECT_EQ("1", ...)` 고정 | 포맷 정책 미정 | `"01"` vs `"1"` |
| `std::stoi` 무검증 | `"abc"`, `""` → 예외 | 테스트/운영 크래시 |

### 2.2 권장 패턴

| 목적 | 권장 코드 | 비고 |
|------|-----------|------|
| 동등 비교 | `parseChannel(s) == 7` | 단일 정규화 함수 |
| 순서 비교 | `int a = parseChannel(...); int b = ...;` | 선호·검색·업다운 |
| 집합 포함 | `favorites.count(parseChannel(getCurrentCH()))` | `set<int>` |
| Tuner 설정 | `setCH(std::to_string(n))` | Fake는 `"1"`/`"12"` 허용 |
| 테스트 검증 | `EXPECT_EQ(7, parseChannel(tuner->getCurrentCH()))` | 또는 포맷 헬퍼 통일 |

```cpp
// 프로젝트 공통 (테스트·프로덕션)
inline int parseChannel(const std::string& s) {
    return std::stoi(s);  // 0~99; invalid는 Tuner/Fake가 throw
}
inline bool sameChannel(const std::string& a, int expected) {
    return parseChannel(a) == expected;
}
```

### 2.3 Mock / Google Mock

```cpp
EXPECT_CALL(mock, setCH("12"));           // 정확한 문자열 기대 시 포맷 고정 필요
EXPECT_CALL(mock, setCH(::testing::_));   // 포맷 독립 검증 시
// Then: EXPECT_EQ(12, parseChannel(...));
```

### 2.4 README vs 테스트 표기

| 출처 | 채널 1 표기 | 채널 7 표기 |
|------|-------------|-------------|
| README 예시 | “1번” | “7번” |
| `TVControllerTest` | `EXPECT_EQ("1", ...)` | `EXPECT_EQ("7", ...)` |
| `.cursorrules` (참고) | `0`→`1` 입력 → `00`~`09` | 두 자리 정책 |

**QA 결론:** 신규 테스트 추가 시 **한 가지 canonical 표현**을 팀에서 선택하고, `parseChannel` 기반 assertion으로 통일한다. 문자열 리터럴 직접 비교는 회귀 위험이 크다.

---

## 3. 예외·경계값 조건

### 3.1 채널 범위 (0 ~ 99)

| 조건 | 유효 | Tuner/Fake 동작 | Controller 책임 |
|------|------|-----------------|-----------------|
| `0` ~ `99` | O | `setCH` 성공 | 버퍼 적용 후 `setCH` |
| `< 0`, `> 99` | X | `std::invalid_argument` (TunerTest) | 적용 전 `clamp` 또는 throw 정책 결정 |
| 순환 업 | 99 → 0 | FR-05-02 | mod 연산 |
| 순환 다운 | 0 → 99 | FR-05-03 | mod 연산 |

```cpp
constexpr int kMinChannel = 0;
constexpr int kMaxChannel = 99;

void validateChannel(int ch) {
    if (ch < kMinChannel || ch > kMaxChannel)
        throw std::invalid_argument("channel out of range");
}
```

### 3.2 숫자 버튼 입력

| 입력 | 결과 |
|------|------|
| `0` ~ `9` | 버퍼 push |
| `< 0` 또는 `> 9` | `std::invalid_argument` (현 `TVChannelController`) |

### 3.3 세 자리 입력·버퍼 처리 (명세 정리)

README (`4`,`5`,`6` 시나리오)와 현재 구현(`buffer.size() >= 3` → **clear만**)은 다르다.

| 단계 | README 기대 | 현재 `TVChannelController` |
|------|-------------|----------------------------|
| `4`,`5` | 45 적용 | 45 적용 (동일) |
| `6` (3번째) | 45 유지 + digit `6` 잔류 가능 | 버퍼 전체 clear, 45만 유지 |
| `6`+확인 | 6번 이동 | (버퍼 비어 있으면) 동작 없음 |
| 기타 버튼 | `6` 무효화 | `pressOther()` → clear (45 유지) |

**QA 권장 순서:**

1. Characterization test로 **README 시나리오**를 코드화 (Red).
2. 구현 수정 또는 README 정정 중 하나로 **단일 진실 공급원** 확정.
3. `.cursorrules`의 “3자리 이상 → **마지막 두 자리**” 규칙은 README와 별도 — 신규 요구 시 **추가 테스트**로만 도입.

### 3.4 빈·비정상 Tuner 응답

| 상황 | 처리 |
|------|------|
| `getCurrentCH()` 빈 문자열 | `stoi` 예외 — 테스트에서 방지 |
| `seekCH()` 빈 문자열 | 검색 루프 종료 (TunerTest 패턴) |
| favorites / searched empty | 다음 선호: no-op; 검색 기반 업다운: FR-05 모드 |

### 3.5 경계값 테스트 매트릭스 (채널 변경)

| # | 입력 | 기대 (int) | 우선순위 |
|---|------|------------|----------|
| B-01 | `0`+확인 | 0 | 높음 |
| B-02 | `9`+확인 | 9 | 높음 |
| B-03 | `9`,`9` | 99 | 높음 |
| B-04 | `1`,`0` | 10 | 중간 |
| B-05 | `9`,`9`+업 (검색 없음) | 0 (99→0) | FR-05 |
| B-06 | `0`+다운 (검색 없음) | 99 | FR-05 |

---

## 4. Google Test 테스트 시나리오 목록

형식: **TS-번호** — Given / When / Then — (구현 상태)

### 4.1 Tuner (Mock) — `test/TunerTest.cpp`

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-001 | 초기 채널 유효 범위 | Mock `getCurrentCH()` → `"0"` | 조회 | 0 ≤ ch ≤ 99 | 구현됨 |
| TS-002 | 유효 채널 set/get | Param: `0`,`4`,`5`,`12`,`99` | `setCH` / `getCurrentCH` | 동일 문자열 반환 | 구현됨 |
| TS-003 | 무효 채널 set | Param: `-12`,`100`, … | `setCH` | `invalid_argument` | 구현됨 |
| TS-004 | seekCH 10회 | Mock 반복 반환 | 10× `seekCH()` | 10개, 각 0~99 | 구현됨 |
| TS-005 | 99에서 seek | `setCH("99")` 후 seek | 10× `seekCH()` | 10개 수집 | 구현됨 |

### 4.2 채널 변경 — `test/TVControllerTest.cpp`

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-101 | 한 자리 + 확인 | 초기 0 | `1`, 확인 | ch == 1 | 구현됨 |
| TS-102 | 두 자리 자동 | 초기 0 | `1`,`2` | ch == 12 | 구현됨 |
| TS-103 | 연속 두 자리 ×2 | 초기 0 | `1`,`2`,`3`,`4` | ch == 34 | 구현됨 |
| TS-104 | 3자리+기타 버튼 | 초기 0 | `4`,`5`,`6`, 기타 | ch == 45 | 구현됨 (README 6 잔류는 미검증) |
| TS-105 | `0`,`7` | 초기 0 | `0`,`7` | ch == 7 | 구현됨 |
| TS-106 | 3자리 후 `6`+확인 | ch==45 | `6`, 확인 | ch == 6 | **미작성 (README)** |
| TS-107 | 한 자리 `0`+확인 | 초기 0 | `0`, 확인 | ch == 0 | 미작성 |
| TS-108 | 경계 `9`,`9` | 초기 0 | `9`,`9` | ch == 99 | 미작성 |

### 4.3 선호 채널

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-201 | 선호 추가 | ch=12, 비선호 | 선호추가 | favorites에 12 | 구현됨 |
| TS-202 | 선호 토글 삭제 | ch=12, 선호 | 선호추가 ×2 | favorites에 12 없음 | 구현됨 |
| TS-203 | 복수 토글 | 12,8,37,8,6 순서 | 선호추가 반복 | {6,12,37} | 구현됨 |
| TS-204 | ch=0 선호 | ch=0 | 선호추가 | favorites에 0 | 미작성 |
| TS-205 | ch=99 선호 | ch=99 | 선호추가 | favorites에 99 | 미작성 |

### 4.4 다음 선호 채널

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-301 | 다음 선호 (일반) | fav {1,4,12,56}, cur=6 | 다음선호 | ch == 12 | 구현됨 |
| TS-302 | wrap | fav {1,56}, cur=56 | 다음선호 | ch == 1 | 구현됨 |
| TS-303 | 빈 목록 | fav ∅, cur=6 | 다음선호 | ch == 6 | 구현됨 |
| TS-304 | cur == 선호 최대 | fav {1,4,12,56}, cur=56 | 다음선호 | ch == 1 | TS-302와 중복 가능 |
| TS-305 | cur가 선호와 동일 | fav {6,12}, cur=6 | 다음선호 | ch == 12 (strict greater) | 미작성 |

### 4.5 채널 검색

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-401 | 전체 검색 저장 | Mock seek 순서 정의 | 채널검색 | 내부 목록 == 기대 | **미구현** |
| TS-402 | seek 종료 | seek → `""` | 채널검색 | 루프 종료, 부분 목록 | **미구현** |
| TS-403 | 검색 후 업/다운 모드 전환 | 검색 완료 | 채널 업 | FR-06 규칙 적용 | **미구현** |

### 4.6 채널 업 (검색 결과 없음)

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-501 | 일반 업 | cur=6, 검색 ∅ | 업 | ch == 7 | **미구현** |
| TS-502 | 99에서 업 wrap | cur=99, 검색 ∅ | 업 | ch == 0 | **미구현** |

### 4.7 채널 다운 (검색 결과 없음)

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-601 | 일반 다운 | cur=6, 검색 ∅ | 다운 | ch == 5 | **미구현** |
| TS-602 | 0에서 다운 wrap | cur=0, 검색 ∅ | 다운 | ch == 99 | **미구현** |

### 4.8 채널 업 (검색 결과 있음)

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-701 | 목록 내 업 | searched {4,6,14}, cur=6 | 업 | ch == 14 | **미구현** |
| TS-702 | 목록 밖 업 wrap | searched {4,6,14}, cur=15 | 업 | ch == 4 | **미구현** |

### 4.9 채널 다운 (검색 결과 있음)

| 번호 | 시나리오 | Given | When | Then | 상태 |
|------|----------|-------|------|------|------|
| TS-801 | 목록 내 다운 | searched {4,6,14}, cur=6 | 다운 | ch == 4 | **미구현** |
| TS-802 | 목록 밖 다운 wrap | searched {4,6,14}, cur=15 | 다운 | ch == 14 | **미구현** |

### 4.10 테스트 작성 규칙 (체크리스트)

1. `TEST_F(ControllerTest, ...)` — 픽스처: `FakeTuner` + `TVChannelController`
2. Given-When-Then 주석 (`// Given`, `// When`, `// Then`)
3. 채널 assert: `parseChannel` 또는 팀 합의 포맷 하나로 통일
4. 검증 채널마다 **별도 TEST_F** (`.cursorrules` / TDD 규칙)
5. 동작 변경 전 테스트 추가 → Green 유지 후 리팩토링
6. `TVController`, `Tuner`, `remoteKey` **수정 금지** — 확장은 `TVChannelController`·테스트 헬퍼

### 4.11 시나리오 요약

| 구분 | 구현됨 | 미작성/미구현 |
|------|--------|----------------|
| Tuner Mock (TS-001~005) | 5 | 0 |
| 채널 변경 (TS-101~) | 5 | 3+ |
| 선호 (TS-201~) | 3 | 2+ |
| 다음 선호 (TS-301~) | 3 | 1+ |
| 검색·업·다운 (TS-401~) | 0 | 12 |

---

## 5. 요구사항 추적 (FR ↔ TS)

| FR ID | 기능 | 대표 TS |
|-------|------|---------|
| FR-01 | 숫자 채널 변경 | TS-101 ~ TS-108 |
| FR-02 | 선호 추가/삭제 | TS-201 ~ TS-205 |
| FR-03 | 다음 선호 | TS-301 ~ TS-305 |
| FR-04 | 채널 검색 | TS-401 ~ TS-403 |
| FR-05 | 업/다운 (검색 없음) | TS-501 ~ TS-602 |
| FR-06 | 업/다운 (검색 있음) | TS-701 ~ TS-802 |

---

## 6. 결론 (QA 관점)

1. **도메인은 int, API는 string** — 비교·분기·선호·검색 목록은 정수 정규화 후 처리한다.
2. **세 자리 버퍼(CH-04)** 는 README와 현 구현이 다르므로 TS-106으로 명세를 고정한 뒤 구현한다.
3. **FR-04~06** 은 테스트 시나리오(TS-401~)가 선행되어야 하며, 업/다운은 검색 목록 유무로 분기한다.
4. 레거시 인터페이스를 유지한 채 `TVChannelController`에 상태(`searchedChannels_`)와 메서드를 추가하는 것이 C++17/TDD에 부합한다.

---

*문서 생성: README.md, TDD_TV_Requirements.txt 기준 — 구현 스냅샷: `TVChannelController.h`, `TVControllerTest.cpp`, `TunerTest.cpp`*
