# 아두이노 스텝모터 거미 알림 장치 프로젝트 플랜 (요약)

상세 프로젝트 플랜은 `archive/spider_project_plan_archived.md`에 아카이브되어 있습니다.

프로젝트 유지 관리 지침:
- 문서의 단일 출처는 `README.md`입니다. README를 편집하면 프로젝트 문서의 주 내용이 갱신됩니다.
- 아카이브된 파일은 참조용으로 보관되며, 필요한 경우 README로 내용을 병합 후 아카이브를 업데이트하세요.
# 아두이노 스텝모터 거미 알림 장치 설계 (.md 정리)

퇴근/출근을 잊지 않도록, 정해진 시간대에 **줄에 매달린 거미**가
살아있는 것처럼 오르내리며 사람 눈에 띄게 만드는 프로젝트 정리본입니다.

- **컨트롤러**: Arduino Uno 또는 Nano
- **모터**: 28BYJ-48 스텝모터 + ULN2003 드라이버 보드
- **시간 관리**: DS3231 RTC 모듈
- **구현 기능**:
  - 아침: **09:20 ~ 09:40**
  - 저녁: **18:00 ~ 18:20**
  - 위 시간대에만 스텝모터가 랜덤 패턴으로 거미를 위/아래로 움직임
  - 한 번의 "세션" 동안 1~3개의 패턴이 연속 실행
  - 패턴 사이에는 진짜 거미처럼 일정 시간 정지 상태 유지
  - 시간대 밖에서는 거의 가만히 있고, 필요하면 서서히 위로 복귀

---

## 1. 전체 시스템 개념

1. 거미 인형을 얇은 실(낚싯줄, 테그줄 등)에 매달고,
2. 실의 윗쪽을 **릴(드럼)**에 감아서 28BYJ-48 스텝모터 축에 고정한다.
3. 스텝모터가 정방향/역방향으로 회전하면서 줄을 감고 풀어
   - 거미가 아래로 내려왔다가
   - 다시 위로 올라가게 만든다.
4. **DS3231 RTC 모듈**로 정확한 현재 시간을 읽어서
   - 오전 9:20~9:40
   - 오후 18:00~18:20
   에만 거미가 "살아있는 것처럼" 랜덤 패턴으로 움직인다.

릴의 반지름은 약 **1.5 ~ 2cm** 정도로 설정하고,
`maxDownSteps` 값(대략 2000~2200 스텝)을 조정하여
실제 거미가 약 **10cm 이상** 내려오도록 튜닝할 수 있다.

---

## 2. 필요 부품 목록

- Arduino Uno 또는 Arduino Nano
- 28BYJ-48 스텝모터
- ULN2003 스텝모터 드라이버 보드
- DS3231 RTC 모듈
- 브레드보드, 점퍼 케이블
- 5V 전원 (USB 어댑터 또는 5V 1A 정도의 어댑터)
- 낚싯줄 / 얇은 실 / 테그줄
- 가벼운 거미 장난감 또는 피규어
- (옵션) 마지막 퇴근 때 거미를 완전 원위치 시키기 위한 버튼 1개

---

## 3. 회로도/배선 설명

### 3.1. DS3231 RTC 모듈 ↔ Arduino

| DS3231 핀 | Arduino 핀 | 기능             |
|----------|------------|------------------|
| VCC      | 5V         | 전원 공급        |
| GND      | GND        | 공통 접지        |
| SDA      | A4         | I²C 데이터 라인  |
| SCL      | A5         | I²C 클럭 라인    |

Arduino Uno/Nano 기준으로 SDA=A4, SCL=A5를 사용한다.

---

### 3.2. 28BYJ-48 + ULN2003 ↔ Arduino

1. **스텝모터 ↔ ULN2003 보드**  
   - 28BYJ-48 모터의 흰색 커넥터를 ULN2003 드라이버 보드의 5핀 소켓에 그대로 꽂는다.

2. **ULN2003 보드 ↔ Arduino**  

| ULN2003 보드 핀 | Arduino 핀 | 설명                  |
|-----------------|------------|-----------------------|
| IN1             | D8         | 제어 신호 1           |
| IN2             | D9         | 제어 신호 2           |
| IN3             | D10        | 제어 신호 3           |
| IN4             | D11        | 제어 신호 4           |
| + (VCC)         | 5V         | 모터 전원 5V          |
| – (GND)         | GND        | 공통 GND              |

3. **전원**

- 가장 간단한 방법: **Arduino의 5V 핀 하나로 RTC + ULN2003 모두 공급**
  - Arduino 5V → RTC VCC, ULN2003 VCC(+)
  - Arduino GND → RTC GND, ULN2003 GND(–)
- 더 안전하게 하려면 외부 5V 어댑터 사용 후
  - ULN2003 VCC와 Arduino 5V 쪽 모두에 공급하고
  - **GND를 공통으로 묶어야 한다**.

> 핵심: 전원은 여러 군데에서 와도 되지만, **GND(접지)는 반드시 공통**이어야 한다.

---

### 3.3. (옵션) 버튼 추가

나중에 마지막 퇴근자가 눌러서 거미를 완전 위로 올리는 기능을 넣고 싶다면,
버튼을 다음처럼 연결한다.

| 버튼 한쪽 | 버튼 다른쪽 |
|----------|-------------|
| D2       | GND         |

코드에서는 내부 풀업 저항을 사용한다.

```cpp
pinMode(2, INPUT_PULLUP);
int buttonState = digitalRead(2); // LOW면 버튼 눌림
```

---

## 4. 시간대별 동작 규칙

### 활성 시간대

- **아침**: 09:20 ~ 09:40  
- **저녁**: 18:00 ~ 18:20  

이 시간대에만 거미가 **살아있는 모드**로 동작한다.

### 세션/패턴 개념

1. 활성 시간대 동안에 일정 간격으로 동작을 시도한다.
2. `nextActionTime`(millis 기반 예약 시간)이 지나면 새로운 "세션" 시작:
   - 이번 세션에서 실행할 패턴 개수를 `1~3개` 랜덤으로 결정
   - 각 패턴을 순서대로 실행
   - 각 패턴과 패턴 사이에 **1~5초 랜덤 정지** (멈춰 있는 거미 느낌)
3. 한 세션이 끝난 뒤,
   - 다음 세션까지 **10~40초 랜덤 휴식** 후 다시 패턴 실행

### 비활성 시간대

- 09:20~09:40, 18:00~18:20 밖의 시간에는
  - 거미는 거의 움직이지 않고,
  - 필요하다면 `currentSteps > 0`일 때 조금씩 위로 올려서
    최대한 상단 근처에 위치하도록 만들어 둘 수 있다.

---

## 5. 스텝/거리 개념

- 28BYJ-48은 보통 **2048스텝 ≈ 한 바퀴**로 사용한다.
- 릴(드럼)의 반지름을 **약 1.5cm** 정도로 잡으면,
  - 둘레 ≈ 2πr ≈ 9.4cm
  - 1바퀴(2048스텝) ≈ 9.4cm
  - 10cm를 내려오게 하고 싶다면 약 **2000~2200스텝** 정도가 필요하다.

코드에서 `maxDownSteps = 2200` 정도로 설정하고,
실제 설치 후 **실제 거리가 바닥에 닿지 않도록** 조금씩 조정하면 된다.

---

## 6. 패턴 설계

### 공통 상태 변수

- `currentSteps` : 현재 거미 위치(스텝 기준), 0 = 최상단
- `maxDownSteps` : 거미가 내려갈 수 있는 최대 스텝 수

```cpp
long currentSteps = 0;          // 0 = 가장 위
const long maxDownSteps = 2200; // 릴/실에 맞게 조정
```

### 스텝 이동 함수

```cpp
void moveSpiderSteps(long steps) {
  long target = currentSteps + steps;

  if (target < 0) {
    steps = -currentSteps;  // 0보다 위로 못 가게
    target = 0;
  } else if (target > maxDownSteps) {
    steps = maxDownSteps - currentSteps;
    target = maxDownSteps;
  }

  if (steps == 0) return;

  spiderStepper.step(steps);
  currentSteps = target;
}
```

### 패턴 예시들

1. **소심한 꿈틀 (작게 통통)**

```cpp
void patternSmallWiggle() {
  Serial.println("Pattern: Small wiggle");
  int amp = 150; // 진동 크기 (스텝 수)

  for (int i = 0; i < 3; i++) {
    moveSpiderSteps(amp);   // 아래로
    delay(200);
    moveSpiderSteps(-amp);  // 위로
    delay(200);
  }
}
```

2. **갑자기 휙 내려갔다 살짝 올라오기**

```cpp
void patternDropAndSlightUp() {
  Serial.println("Pattern: Drop and slight up");
  int drop = random(400, 700);    // 크게 내려가기
  int up   = random(100, 250);    // 살짝 올라오기

  moveSpiderSteps(drop);   // 아래로
  delay(300);
  moveSpiderSteps(-up);    // 조금 위로
  delay(300);
}
```

3. **위로 슬쩍 올라갔다 원위치 복귀**

```cpp
void patternUpAndBack() {
  Serial.println("Pattern: Up and back");
  int up = random(200, 400);

  moveSpiderSteps(-up);    // 위로
  delay(400);
  moveSpiderSteps(up);     // 다시 아래(원래 위치)
  delay(400);
}
```

4. **긴 숨 고르기 (천천히 기어가기)**

```cpp
void patternSlowMove() {
  Serial.println("Pattern: Slow move");
  int dir = (random(0, 2) == 0) ? 1 : -1;   // 1: 아래, -1: 위
  int dist = random(300, 800);              // 전체 이동 거리

  int stepSize = (dir > 0) ? 10 : -10;      // 한 번에 조금씩
  int stepsMoved = 0;

  while (abs(stepsMoved) < dist) {
    moveSpiderSteps(stepSize);
    stepsMoved += stepSize;
    delay(80);  // 천천히 움직이는 느낌
  }
}
```

5. **랜덤 패턴 선택 함수**

```cpp
void runRandomPattern() {
  int p = random(0, 4); // 0,1,2,3 중 하나

  switch (p) {
    case 0: patternSmallWiggle();      break;
    case 1: patternDropAndSlightUp();  break;
    case 2: patternUpAndBack();        break;
    case 3: patternSlowMove();         break;
  }
}
```

---

## 7. 전체 코드 (아침/저녁 시간대 + 랜덤 세션)

아래 코드는:

- RTC(DS3231) + 28BYJ-48 + ULN2003 + Arduino 기반
- 시간대:
  - **아침 09:20 ~ 09:40**
  - **저녁 18:00 ~ 18:20**
- 해당 시간대에만 "세션" 단위로 랜덤 패턴 실행
- 세션 사이에는 쉬는 시간(정지 상태)을 두어 거미가 진짜처럼 보이게 함
- 비활성 시간대에는 거미가 천천히 위쪽으로 복귀

### 코드

```cpp
#include <Wire.h>
#include "RTClib.h"
#include <Stepper.h>

RTC_DS3231 rtc;

// ===== 스텝모터 설정 =====
const int STEPS_PER_REV = 2048;

// ULN2003 보드 IN1~IN4를 Arduino 8~11번에 연결했다고 가정
const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

// Stepper 라이브러리: (IN1, IN3, IN2, IN4) 순서 많이 사용됨
Stepper spiderStepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);

// ===== 거미 위치 관리 =====
long currentSteps = 0;             // 0 = 가장 위
const long maxDownSteps = 2200;    // 약 10cm 정도 내려오는 한계 (릴/실에 따라 조정)

// ===== 패턴 실행 타이밍 =====
unsigned long nextActionTime = 0;           // 다음 “세션” 시작 예정 시간 (millis 기준)

// ======================== 기본 설정 ========================
void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();

  // 스텝모터 속도 (RPM). 너무 빠르면 토크 부족/미끄러짐.
  spiderStepper.setSpeed(10); // 10RPM 정도로 천천히

  // ★ RTC 시간 최초 1회 설정용 (사용 후에는 꼭 주석 처리!)
  // rtc.adjust(DateTime(2025, 11, 13, 9, 10, 0)); // 연,월,일,시,분,초

  // 랜덤 초기값
  randomSeed(analogRead(A0));

  // 처음엔 5초 뒤쯤 첫 행동 가능하도록
  nextActionTime = millis() + 5000;

  Serial.println("Spider random alive mode with time windows ready!");
}

// ======================== 유틸 함수들 ========================

// 거미 이동: steps > 0 아래로, steps < 0 위로
void moveSpiderSteps(long steps) {
  long target = currentSteps + steps;

  if (target < 0) {
    steps = -currentSteps;  // 0보다 위로 못 가게
    target = 0;
  } else if (target > maxDownSteps) {
    steps = maxDownSteps - currentSteps; // 아래 한계 넘지 않도록
    target = maxDownSteps;
  }

  if (steps == 0) return;

  spiderStepper.step(steps);
  currentSteps = target;
}

// 현재 시간이 활성 시간대인지 확인
bool isActiveTime(DateTime now) {
  int hour = now.hour();
  int minute = now.minute();

  bool morning = false;
  bool evening = false;

  // 아침: 09:20 ~ 09:40
  if (hour == 9 && minute >= 20 && minute <= 40) {
    morning = true;
  }

  // 저녁: 18:00 ~ 18:20
  if (hour == 18 && minute >= 0 && minute <= 20) {
    evening = true;
  }

  return (morning || evening);
}

// ======================== 패턴들 정의 ========================

// 1. 소심한 꿈틀 (작게 통통 뛰기)
void patternSmallWiggle() {
  Serial.println("Pattern: Small wiggle");
  int amp = 150; // 진동 크기 (스텝 수)

  for (int i = 0; i < 3; i++) {
    moveSpiderSteps(amp);   // 아래로
    delay(200);
    moveSpiderSteps(-amp);  // 위로
    delay(200);
  }
}

// 2. 갑자기 휙 내려갔다 살짝 올라오기
void patternDropAndSlightUp() {
  Serial.println("Pattern: Drop and slight up");
  int drop = random(400, 700);    // 크게 내려가기
  int up   = random(100, 250);    // 살짝 올라오기

  moveSpiderSteps(drop);   // 아래로
  delay(300);
  moveSpiderSteps(-up);    // 조금 위로
  delay(300);
}

// 3. 위로 슬쩍 올라갔다 제자리로
void patternUpAndBack() {
  Serial.println("Pattern: Up and back");
  int up = random(200, 400);

  moveSpiderSteps(-up);    // 위로
  delay(400);
  moveSpiderSteps(up);     // 다시 아래(원래 위치)
  delay(400);
}

// 4. 긴 숨 고르기 (천천히 크게 이동: 기어가는 느낌)
void patternSlowMove() {
  Serial.println("Pattern: Slow move");
  int dir = (random(0, 2) == 0) ? 1 : -1;   // 1: 아래, -1: 위
  int dist = random(300, 800);              // 전체 이동 거리

  int stepSize = (dir > 0) ? 10 : -10;      // 한 번에 조금씩
  int stepsMoved = 0;

  while (abs(stepsMoved) < dist) {
    moveSpiderSteps(stepSize);
    stepsMoved += stepSize;
    delay(80);  // 천천히 움직이는 느낌
  }
}

// 랜덤 패턴 하나 실행
void runRandomPattern() {
  int p = random(0, 4); // 0,1,2,3 중 하나

  switch (p) {
    case 0: patternSmallWiggle();      break;
    case 1: patternDropAndSlightUp();  break;
    case 2: patternUpAndBack();        break;
    case 3: patternSlowMove();         break;
  }
}

// ======================== 메인 루프 ========================
void loop() {
  DateTime now = rtc.now();

  bool active = isActiveTime(now);
  unsigned long nowMillis = millis();

  if (active) {
    // 활성 시간대: 아침(9:20~9:40) / 저녁(18:00~18:20)
    if ((long)(nowMillis - nextActionTime) >= 0) {
      // 이번 "세션"에서 실행할 패턴 개수 (1~3개)
      int numPatterns = random(1, 4);

      Serial.print("Session start, patterns: ");
      Serial.println(numPatterns);

      for (int i = 0; i < numPatterns; i++) {
        runRandomPattern();

        // 패턴과 패턴 사이에 정지 시간 (1~5초)
        unsigned long pause = random(1000, 5000);
        Serial.print("Pause between patterns (ms): ");
        Serial.println(pause);
        delay(pause);
      }

      // 세션 끝난 후, 다음 세션까지 쉬는 시간 (10~40초)
      unsigned long rest = random(10000, 40000);
      Serial.print("Rest until next session (ms): ");
      Serial.println(rest);

      nextActionTime = millis() + rest;
    }

  } else {
    // 비활성 시간대: 크게 움직이지 않음
    // 필요하다면, 천천히 위로 복귀시키는 로직 추가
    if (currentSteps > 0) {
      moveSpiderSteps(-10); // 조금씩 위로 올리기
      delay(100);
    }

    // 다음 활성 구간에서 바로 폭주하지 않도록, 여유 시간 설정
    if ((long)(nowMillis - nextActionTime) >= 0) {
      nextActionTime = nowMillis + 10000; // 10초 후부터 행동 가능
    }

    delay(200);
  }
}
```

---

## 8. 앞으로 확장 아이디어

- **퇴근 버튼 연동**  
  - 마지막 퇴근자가 버튼을 누르면 `moveSpiderSteps(-currentSteps);`를 실행하여  
    거미를 완전히 위로 올린 뒤, 그날은 더 이상 움직이지 않게 설정 가능.
- **LED 눈 추가**  
  - 거미 눈 부분에 빨간 LED 2개를 달고  
    패턴 실행 중일 때만 켜지게 하면 시각적 효과 극대화.
- **ESP32로 확장**  
  - Wi-Fi로 NTP 시간 동기화
  - “오늘 거미가 몇 번 움직였는지”를 서버나 구글시트에 기록하는 등의 데이터화도 가능.

이 문서 하나만 가지고 있어도,  
아두이노 + 스텝모터 기반 "살아있는 거미 출근/퇴근 알림 장치"를
다시 구현할 수 있도록 전체 내용을 정리했습니다.
