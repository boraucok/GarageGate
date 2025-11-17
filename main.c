#include "mbed.h"

// Pin tanımlamaları
#define TRIG_PIN D6
#define ECHO_PIN D7
#define SERVO_PIN D9
#define EXIT_TRIG_PIN D8
#define EXIT_ECHO_PIN D10

DigitalOut trigger(TRIG_PIN);
DigitalIn echo(ECHO_PIN);
DigitalOut exitTrigger(EXIT_TRIG_PIN);
DigitalIn exitEcho(EXIT_ECHO_PIN);
PwmOut servo(SERVO_PIN);

// Fonksiyon: Ultrasonik mesafe ölçümü
long readUltrasonicDistance(DigitalOut& trig, DigitalIn& echo) {
    trig = 0;
    wait_us(2);
    trig = 1;
    wait_us(10);
    trig = 0;

    // Yankının yüksek durum süresini ölç
    Timer t;
    t.start();
    while (!echo) {} // Yankının başlamasını bekle
    t.reset();
    while (echo) {}  // Yankının bitmesini bekle
    long duration = t.read_us();

    return duration;
}

void setServoAngle(float angle) {
    // Açıyı PWM darbe genişliğine çevir (örnek aralık: 1ms - 2ms için 0 - 180 derece)
    float pulseWidth = 0.001 + (angle / 180.0) * 0.001;
    servo.pulsewidth(pulseWidth);
}

int main() {
    servo.period(0.02); // 20ms periyot (50Hz)

    while (1) {
        long duration = readUltrasonicDistance(trigger, echo);
        int cm = 0.01723 * duration; // Mesafe hesaplama

        if (cm < 30) { // Girişte sensör algılarsa
            printf("Mesafe: %d cm\n", cm);

            // Servo 0'dan 180 dereceye açılır
            for (float angle = 0.0; angle <= 180.0; angle += 1.0) {
                setServoAngle(angle);
                wait_us(10000);
            }

            // Çıkış sensörünü kontrol et
            printf("Çıkış sensörü bekleniyor...\n");
            while (1) {
                long exitDuration = readUltrasonicDistance(exitTrigger, exitEcho);
                int exitCm = 0.01723 * exitDuration; // Çıkış mesafe hesaplama
                if (exitCm < 30) {
                    break; // Araç çıkış yaptı
                }
                ThisThread::sleep_for(100ms); // Beklemeye devam et
            }

            printf("Araba çıkış yaptı, kapı kapanıyor...\n");
            wait_us(3000000); // 12 saniye bekleme

            // Servo 180'den 0 dereceye kapanır
            for (float angle = 180.0; angle >= 0.0; angle -= 1.0) {
                setServoAngle(angle);
                wait_us(10000);
            }
            wait_us(5000000); // İstediğiniz kadar gecikme
        }
    }
}
