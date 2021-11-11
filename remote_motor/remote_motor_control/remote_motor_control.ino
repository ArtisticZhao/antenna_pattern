/**
 * 协议版本： v2.0
 * 协议内容： 
 *          1. 控制器保存绝对角度
 *          2. 控制器每500ms 回报一次当前角度 格式为 Angle: 9000, 代表当前角度为90°
 *          3. 控制器输入绝对角度 格式为 cmd 9000, 代表命令控制器旋转到绝对角度90.0° 
 */

#define DIR 8
#define PULSE 9
#include <MsTimer2.h>
long current_angle;
void report_angle(){
  Serial.print("Angle: ");
  Serial.print(current_angle);
  Serial.print("\n");
}
void setup() {
  // put your setup code here, to run once:
  pinMode(PULSE, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite(PULSE, LOW);
  digitalWrite(DIR, LOW);
  current_angle = 0;
  
  MsTimer2::set(500, report_angle);        // 中断设置函数，每 500ms 进入一次中断
  MsTimer2::start();                //开始计时

  // set serial port
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available()){
    String str = Serial.readString();
    if (str.equals("rst")){
      current_angle = 0;
    }else{
      String str_angle = getValue(str, ' ', 1);
      long angle = str_angle.toInt();
      long d_angle = angle - current_angle;
      if (d_angle > 0){
        ratote(d_angle, 1);
      }else if(d_angle < 0){
        ratote(-d_angle, 0);
      }
    }
  }
}

void ratote(long angle, long dir){
  MsTimer2::stop();
  long r_times = angle / 72;
  if (dir == 1) {
    digitalWrite(DIR, LOW);
    current_angle += angle;
  }else {
    digitalWrite(DIR, HIGH);
    current_angle -= angle;
  }
  for (long i=0; i<r_times; i++){
    step(2);
  }
  MsTimer2::start();
}

void step(long d_time){
  digitalWrite(PULSE, HIGH);
  delay(d_time);
  digitalWrite(PULSE, LOW);
  delay(d_time);
}

String getValue(String data, char separator, long index)
{
  long found = 0;
  long strIndex[] = {0, -1};
  long maxIndex = data.length()-1;

  for(long i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
