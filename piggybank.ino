//아두이노 저금통 (창의적 공학 가반 우노)
//참고 자료 https://www.youtube.com/c/COMMONCODING/featured

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <Servo.h> 


void MoneyWrite(int money); // 금액의 값을 내장 메모리에 저장해두는 함수
unsigned long MoneyRead(); // 내장 메모리에 저장된 금액을 불러오는 함수
void MoneyReset(); // 내장 메모리에 저장된 금액을 초기화 하는 함수
unsigned long pow_int(); // double형이 아닌 int형 제곱식을 위한 함수
void setLocked(int locked); // 서브모터를 회전시켜 문을 잠그거나 푸는 함수 

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7); // lcd 객체 생성

const int ir_10 = 22;    // 10,50,100,500원 동전을 인식할 적외선 센서 4개 
const int ir_50 = 23;
const int ir_100 = 24;
const int ir_500 = 25;
unsigned long money; // 금액을 저장할 전역변수

Servo servo; // 서브모터 객체
String passWord = "2837"; //비밀번호 설정

int position = 0; // 비밀번호가 한자리씩 맞을때마다 값을 증가시킬 변수

char keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
}; // 키패드 버튼 위치와 똑같이 배열을 생성

byte rowPins[4]={9,8,7,6}; //키패드에 연결된 핀번호 설정
byte colPins[4]={5,4,3,2};

Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,4,4); // 키패드 객체 생성

void setup() {
  Serial.begin(9600);
  pinMode(ir_10, INPUT);
  pinMode(ir_50, INPUT);
  pinMode(ir_100, INPUT);
  pinMode(ir_500, INPUT);
  lcd.setBacklightPin(3,POSITIVE); // lcd light 설정
  lcd.setBacklight(LOW); //lcd light 설정
  lcd.begin(16, 2);
  //MoneyReset();  
  money = MoneyRead(); // 메모리에 저장되있던 금액을 불러옴 

  servo.attach(10); //서보모터 10번핀에 연결
  setLocked(true); // 문을 잠근다.
}

void loop() {
  if (digitalRead(ir_10) == LOW) // 10원에 해당하는 적외선 센서
  {
    money+=10; // 10원을 sum에 더함
    MoneyWrite(money);  // 메모리에 금액을 저장
    Serial.println("10");
    delay(100);
  }
  if (digitalRead(ir_50) == LOW) // 50원에 해당하는 적외선 센서
  {
    money+=40; // 40원을 sum에 더함
    MoneyWrite(money); // 메모리에 금액을 저장
    Serial.println("50");
    delay(25);
  }
  if (digitalRead(ir_100) == LOW) // 100원에 해당하는 적외선 센 서
  {
    money+=50; // 50원을 sum에 더함
    MoneyWrite(money); // 메모리에 금액을 저장
    Serial.println("100");  
    delay(25);
  }
  if (digitalRead(ir_500) == LOW) // 500원에 해당하는 적외선 센서
  {
    money+=400; // 400원을 sum에 더함
    MoneyWrite(money); // 메모리에 금액을 저장
    Serial.println("500");
    delay(100);
  }
  
  lcd.setCursor(0, 0); // 금액 lcd로 출력
  lcd.print("MONEY:");
  lcd.setCursor(0, 1); 
  lcd.print(money); 

  char key = keypad.getKey(); // 키패드에서 입력된 값 저장

  Serial.println(key); 

  if((key >= '0' && key<= '9') || (key >= 'A' && key <= 'D') || (key=='*' || key == '#'))
    {
       if(key == '*' || key == '#') // *,# 을 누르면 잠근다.
       {
        position = 0;  
        setLocked(true);
       }

       else if(key == passWord[position]) // 맞는 번호를 입력할 경우 position 증가
       {
        position++;  
       }

       else if(key != passWord[position]) // 틀린 번호를 입력할 경우 postion 0으로 초기화하고 문을 잠근다.
       {
        position=0;
        setLocked(true);
       }

       if(position ==4) // 비밀번호 4자리가 알맞게 입력되면 position이 4 상태이므로 문 잠금을 해제한다.
         setLocked(false);
      }
      
      delay(50);
}

void MoneyWrite(unsigned long money)
{
  int i=0;
  
  for(;money!=0;i++)
  {
    EEPROM.write(i,money%10); //메모리 0번지부터 금액의 맨 마지막 자리 숫자 저장 
    money/=10; // 금액의 맨 마지막 자리 숫자를 변경하기 위해 10으로 나눈다.
  }

  EEPROM.write(i,10); // 마지막으로 값이 저장된 메모리 공간의  
                      //   바로 다음 공간에 10을 넣어서 금액의 끝을 알 수 있게 한다. 
}

unsigned long MoneyRead()
{

  unsigned long money=0;
  
  for(int i=0; EEPROM.read(i)!=10 ;i++)
  {
    if(EEPROM.read(i)!=0)
    {
      money+= EEPROM.read(i) * pow_int(10,i); //메모리 0번지부터 한 자리씩 10^i 승을 곱해서
                                              //저장된 금액을 10진수로 변환한다.
    }
  }

  return money;
}

void MoneyReset()
{
   EEPROM.write(0,10);
  
  for(int i=1;i<10;i++) 
    EEPROM.write(i,0); // 내부 메모리 공간을 초기화한다. (저장된 금액 초기화)
  // 원래는 EEPROM 공간이 1kb이므로 1023까지 보통 돌리지만 10자리 이하 정도만 표현하니 상관없다.  
}

unsigned long pow_int(int x,int y) // double형이 아닌 int형 제곱식을 위한 함수
{
  int result=1;
  
  for(int i=0;i<y;i++)
    result*=x;

    return result;
}

void setLocked(int locked)
{
  if(locked) //잠긴 상태면
  {
    servo.write(0); //서보를 0도로 회전시켜서 문을 잠근다.
  }

   else // 잠금을 해제한 상태면
   {
    servo.write(90); //서보를 90도로 회전시켜서 문 잠금을 해제한다.
   }
}
