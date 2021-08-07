#define DIR 8
#define PULSE 9
void setup() {
  // put your setup code here, to run once:
  pinMode(PULSE, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite(PULSE, LOW);
  digitalWrite(DIR, LOW);

  // set serial port
  Serial.begin(115200);
  Serial.print("\n");
  Serial.print("Usage: cmd <angle*100> <direction>\n");
  Serial.print("e.g.: Turn 90 degress clockwise # cmd 9000 0\n");
  Serial.print("\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available()){
    String str = Serial.readString();
    Serial.print(str);
    String str_angle = getValue(str, ' ', 1);
    int angle = str_angle.toInt();
    String str_dir = getValue(str, ' ', 2);
    int dir = str_dir.toInt();
    Serial.print("angle: ");
    Serial.print(angle);
    Serial.print(" ");
    Serial.print("dir: ");
    Serial.print(dir);
    Serial.print("\n");
    
    ratote(angle, dir);
    Serial.print("OK\n");
  }
}

void ratote(int angle, int dir){
  int r_times = angle / 72;
  if (dir == 1) {
    digitalWrite(DIR, LOW);
  }else {
    digitalWrite(DIR, HIGH);
  }
  for (int i=0; i<r_times; i++){
    step(2);
  }
}

void step(int d_time){
  digitalWrite(PULSE, HIGH);
  delay(d_time);
  digitalWrite(PULSE, LOW);
  delay(d_time);
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
