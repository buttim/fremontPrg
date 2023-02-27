#define VCC   5
#define CLOCK 6
#define DATA  7

#define N 32
#define DELAY 2

enum Opcode {
  RESET_ADDRESS_POINTER=4,
  INCREMENT_ADDRESS_POINTER=5,
  STORE_LSB=8,
  STORE_MSB=9,
  CHANGE_MODE=0x0A,
  READ_LSB=0x0C,
  READ_MSB=0x0D
};

enum Mode {
  FLASH_READ=0x20,
  FLASH_WRITE=0x30
};

void serialWrite(uint16_t data,uint8_t count) {
  digitalWrite(CLOCK,HIGH);
  pinMode(DATA,OUTPUT);
  digitalWrite(2,HIGH);///////////////////
  for (int i=0;i<count;i++) {
    digitalWrite(DATA,((data&1)==0)?LOW:HIGH);
    digitalWrite(CLOCK,LOW);
    delayMicroseconds(DELAY);
    digitalWrite(CLOCK,HIGH);
    delayMicroseconds(DELAY);
    data>>=1;
  }
}

uint8_t serialRead(uint8_t count) {
  uint8_t result=0,bit=1;

  digitalWrite(CLOCK,HIGH);
  digitalWrite(DATA,LOW);
  pinMode(DATA,INPUT);
  digitalWrite(2,LOW);///////////////////
  for (int i=0;i<count;i++) {
    delayMicroseconds(DELAY);
    digitalWrite(CLOCK,LOW);
    delayMicroseconds(DELAY);
    result|=(digitalRead(DATA)==HIGH)?bit:0;
    digitalWrite(CLOCK,HIGH);
    bit<<=1;
  }
  return result;
}

void enterProg() {
  digitalWrite(VCC,HIGH);
  delay(1);
  serialWrite(0x1CA3,16);
  delay(70);
}

void opExecute(Opcode opcode) {
  serialWrite(opcode,5);
}

void opWrite(Opcode opcode,uint8_t parameter) {
  serialWrite(opcode,5);
  serialWrite(parameter,8);
  serialWrite(0,1);
}

uint8_t opRead(Opcode opcode) {
  serialWrite(opcode,5);
  uint8_t response=serialRead(8);
  serialRead(1);
  return response;
}

void eraseFlash(){
  opWrite(CHANGE_MODE, FLASH_READ);
  opWrite(CHANGE_MODE, 0x25);
  opWrite((Opcode)0x9, 0x1A);
  opWrite((Opcode)0x8, 0x08);
  opExecute((Opcode)0x6);
  opExecute((Opcode)0x4);
  opWrite(CHANGE_MODE, 0x31);
  opExecute((Opcode)0x2);

  //Wait until memory is erased
  delay(50);

  //???
  opExecute((Opcode)0x7);
}

void setup() {
  int i;
  
  Serial.begin(115200);
  Serial.println("START");

  pinMode(2,OUTPUT);//////////

  pinMode(CLOCK,OUTPUT);
  digitalWrite(CLOCK,LOW);

  pinMode(DATA,OUTPUT);
  digitalWrite(DATA,LOW);
  pinMode(VCC,OUTPUT);
  digitalWrite(VCC,LOW);

  delay(100);

  enterProg();

  //eraseFlash();

  //write
  opWrite(CHANGE_MODE, 0x56);
  opWrite(CHANGE_MODE, FLASH_READ);
  opWrite(CHANGE_MODE, 0x24);
  opWrite((Opcode)0x9, 0x1A);
  opWrite((Opcode)0x8, 0x00);
  opExecute((Opcode)0x6);

  opExecute(RESET_ADDRESS_POINTER);

  for (i=0;i<N;i++) {
    uint16_t data=i;
    opWrite(CHANGE_MODE,FLASH_WRITE);
    opWrite(STORE_MSB,(data>>7)&0x7F);
    opWrite(STORE_LSB,data&0x7F);

    opExecute((Opcode)0x1);
    opExecute((Opcode)0x6);

    delayMicroseconds(1500);

    opExecute((Opcode)0x7);
    
    opExecute(INCREMENT_ADDRESS_POINTER);
  }

  delay(150);

  //read
  opWrite(CHANGE_MODE,FLASH_READ);
  opExecute(RESET_ADDRESS_POINTER);
  for (i=0;i<N;i++) {
    uint8_t lsb=opRead(READ_LSB)&0x7F;
    uint8_t msb=opRead(READ_MSB)&0x7F;
    Serial.print((msb<<7) | lsb, HEX);
    opExecute(INCREMENT_ADDRESS_POINTER);
    Serial.print(i%16==15?'\n':' ');
  }

  digitalWrite(VCC,LOW);
  pinMode(DATA,OUTPUT);
  digitalWrite(DATA,LOW);
  digitalWrite(CLOCK,LOW);

  Serial.println("\nFINE");
}

void loop() {
}
