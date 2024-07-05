
#include <HardwareSerial.h>

#define simSerial               Serial2
#define MCU_SIM_BAUDRATE        115200
#define MCU_SIM_TX_PIN          17
#define MCU_SIM_RX_PIN          16
#define MCU_SIM_EN_PIN          15

// Định nghĩa chân pin cảm biến lửa và MQ2
#define FLAME_PIN               18
#define MQ2_PIN                 32

// Ngưỡng giá trị cho cảm biến MQ2 và cảm biến lửa
#define MQ2_THRESHOLD           800
#define FLAME_THRESHOLD         LOW // Giả sử khi chân pin cảm biến lửa xuất hiện mức LOW là phát hiện lửa

#define PHONE_NUMBER            "0374766875"

void sim_at_wait()
{
    delay(100);
    while (simSerial.available()) {
        Serial.write(simSerial.read());
    }
}

bool sim_at_cmd(String cmd) {
    simSerial.println(cmd);
    sim_at_wait();
    return true;
}


bool sim_at_send(char c){
    simSerial.write(c);
    return true;
}

void sent_sms(String message) {
    sim_at_cmd("AT+CMGF=1"); // Chế độ văn bản
    String temp = "AT+CMGS=\"";
    temp += PHONE_NUMBER;
    temp += "\"";
    sim_at_cmd(temp);
    sim_at_cmd(message); // Nội dung tin nhắn

    // Kết thúc tin nhắn
    sim_at_send(0x1A);
}



void call() {
    String temp = "ATD";
    temp += PHONE_NUMBER;
    temp += ";";
    sim_at_cmd(temp); // Gọi đi

    delay(20000); // Đợi 20 giây

    sim_at_cmd("ATH"); // Cúp máy
}



void setup() 
{
    /*  Bật nguồn mô-đun SIM  */
    pinMode(MCU_SIM_EN_PIN, OUTPUT); 
    digitalWrite(MCU_SIM_EN_PIN, LOW);

    delay(20);
    Serial.begin(115200);
    Serial.println("\n\n\n\n-----------------------\nHệ thống bắt đầu!!!!");

    // Đợi 8 giây để mô-đun SIM khởi động
    delay(8000);
    simSerial.begin(MCU_SIM_BAUDRATE, SERIAL_8N1, MCU_SIM_RX_PIN, MCU_SIM_TX_PIN);

    // Kiểm tra lệnh AT
    sim_at_cmd("AT");

    // Thông tin sản phẩm
    sim_at_cmd("ATI");

    // Kiểm tra khe SIM
    sim_at_cmd("AT+CPIN?");

    // Kiểm tra chất lượng tín hiệu
    sim_at_cmd("AT+CSQ");

    sim_at_cmd("AT+CIMI");

    // Khai báo chân pin cảm biến lửa là đầu vào
    pinMode(FLAME_PIN, INPUT); 

    // Khai báo chân pin cảm biến MQ2 là đầu vào
    pinMode(MQ2_PIN, INPUT); 


    // Đợi 5 giây
    delay(5000);   
}

void loop() 
{     
    // Kiểm tra phát hiện lửa từ cảm biến lửa
    int flame_detected = digitalRead(FLAME_PIN);
    Serial.print("Trạng thái cảm biến lửa: ");
    Serial.println(flame_detected);
    delay (1000);
    if (flame_detected == FLAME_THRESHOLD) {
        Serial.println("Cảnh báo, Phát hiện lửa");
        sent_sms("Fire Detected");

        delay(5000); // Chờ đợi cho tin nhắn được gửi đi

        call(); // Thực hiện cuộc gọi
    } 

    // Kiểm tra phát hiện khí gas từ cảm biến MQ2
    int MQ2_value = analogRead(MQ2_PIN);
    Serial.print("Giá trị MQ2: ");
    Serial.println(MQ2_value);
    delay (1000);
    if (MQ2_value > MQ2_THRESHOLD) {
        Serial.println("Phát hiện rò rỉ khí gas");
        sent_sms("Gas leak");

        delay(5000); // Chờ đợi cho tin nhắn được gửi đi

        call(); // Thực hiện cuộc gọi
    } 

    // Giao tiếp với mô-đun SIM qua cổng Serial
    if (Serial.available()){
        char c = Serial.read();
        simSerial.write(c);
    }
    sim_at_wait();
}
