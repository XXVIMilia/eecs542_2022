#include <Arduino.h>
#include <TimerOne.h>


//Signal Detection Variables
volatile unsigned long fallTime = 0;
volatile unsigned long prevTime = 0;
volatile unsigned long sampledDelays[20];
volatile bool trigger = 0;
volatile int count = 0;
volatile bool checkDelay = 0;
unsigned long detectedBitRate = 0;
volatile bool calibrating = 0;
volatile bool hit;

//Timing Verification Variables
unsigned long prevTimeStamp;
unsigned long driftCorrection;


//Main Variables
bool detectedSignal = 0;
bool validatedSignal = 0;
bool recievingMode = 0;

//Message reading
volatile int incomingPacket[14];
volatile int readCount = 0;




//User input
char command;

//fall Time Detection Callback Function
void fall(){
  if(calibrating){
    prevTime = fallTime;
    fallTime = micros();
    sampledDelays[count % 20] = fallTime - prevTime;
    count++;
  }
  
}

//Function that takes sampled bit delays and averages them
void calibrateDelay(){
  count = 0;
  calibrating = 1;
  attachInterrupt(digitalPinToInterrupt(2),fall,FALLING);
  while(count < 20){
    //Busy Wait
  }
  calibrating = 0;
  detachInterrupt(digitalPinToInterrupt(2));

  unsigned long sum = 0;
  for(int i = 5; i < 20; i++){
    Serial.println(sampledDelays[i]);
    sum += sampledDelays[i];
  }
  
  detectedBitRate = sum / 15;

}


void readData(){
    if(trigger){
      //if(readCount > 0){//Ignore first bit in stream
        
        digitalWrite(7, LOW);
        if(hit){
          incomingPacket[readCount] = 1;
        }
        else{
          incomingPacket[readCount] = 0;
        }
        
      //}
      

      hit = 0;
      if(readCount == 14){
        readCount = 0;
        trigger = 0;
      }
      digitalWrite(7, HIGH);

      readCount++;
    }
    
    
    
}

//Simple cleanup function
void resetArrays(){
  for(int i = 0; i < 20; i++){
    sampledDelays[i] = 0;
  }
  for(int i = 0; i < 14; i++){
    incomingPacket[i] = 0;
  }
}

volatile int hitsCounted;
volatile bool awaitTrigger = 0;
//Use for getting exact start of signal tranmsission. Used for detecting falling edge of bits
void awaitTriggerSignal(){
  if(awaitTrigger){
    digitalWrite(3,LOW);
    if(!trigger){
      trigger = 1;
      // Timer1.restart();
      hit = 0;
      hitsCounted = 0;
      
    }
    else{
      hit = 1;
      hitsCounted++;
      
    }  

    digitalWrite(3, HIGH);

    }
  
  
}



//Verifies the calculated bit delay is usable
bool verifySignal(){
  trigger = 0;
  readCount = 0;
  hit = 0;

   
  awaitTrigger = 1;
  while(!trigger){}


  
  while(trigger){}
  awaitTrigger = 0;

  digitalWrite(3, HIGH);
  digitalWrite(7, HIGH);


  for(int i = 1; i < 14; i++){
    if(!incomingPacket[i]){ 
      return(false);
    }
  }
  
  return(true);
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2,INPUT);
  pinMode(3,OUTPUT);
  pinMode(7,OUTPUT);
  digitalWrite(3, HIGH);
  digitalWrite(7, HIGH);
  Timer1.initialize();
  
}



bool timerActive = 0;
void loop() {
  resetArrays();
  Serial.println("Send a command: 'c' = calibrate, 'r' = read");
  while(!Serial.available()){}
  command = Serial.read(); 

  if(command == 'c'){
    Serial.println("Attempting to detect signal");
    calibrateDelay();
    attachInterrupt(digitalPinToInterrupt(2),awaitTriggerSignal,FALLING);
    Serial.print("Detected Bit Rate: ");
    Serial.println(detectedBitRate);
    Timer1.attachInterrupt(readData,detectedBitRate);
    Timer1.start();
    timerActive = 1;


    //Validating
    Serial.println("Attempting to validate signal");
    validatedSignal = verifySignal();
    if(validatedSignal){
      Serial.println("Signal Verified");
      recievingMode = 1;
    }
    else{
      Serial.println("Verify Failed. Trying again"); 
      detectedSignal = 0;
      recievingMode = 0;
    }

    Serial.println("Packet detected");
    for(int i = 1; i < 14; i++){
      Serial.print(incomingPacket[i]);
    }
    Serial.print('\n');
    //do{}while(readMessage());//Trying to fix reading bug. Ugly solution
  }

  
}