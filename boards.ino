
#include <SPI.h>
#include "printf.h"
#include "RF24.h"


// Notas
// 2ª Oitava
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123

// 3ª Oitava
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247

// 4ª Oitava (Completa)
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
// 5ª Oitava
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988

// USO para o buzzer
#define BUZZER_PIN 3

//Uso para o RF24
#define CE_PIN 7
#define CSN_PIN 8

// Uso dos sensor de presença
#define led_PIN 5
#define sensor_PIN 6
int leitura = 0;            // armazenar a leitura do sensor
bool estadoSensor = false;  // armazenar o estado do sensor

int mario_melody[] = {

  NOTE_E5, NOTE_E5, 0, NOTE_E5,
  0, NOTE_C5, NOTE_E5, 0,
  NOTE_G5, 0, 0, 0,
  NOTE_G4, 0, 0, 0,

  NOTE_C5, 0, 0, NOTE_G4,
  0, 0, NOTE_E4, 0,
  0, NOTE_A4, 0, NOTE_B4,
  0, NOTE_A4, NOTE_G4, 0,

  NOTE_E5, NOTE_G5, NOTE_A5,
  0, NOTE_F5, NOTE_G5,
  0, NOTE_E5, 0, NOTE_C5,
  NOTE_D5, NOTE_B4
};


int duration[] = {

  150, 150, 150, 150,
  150, 150, 150, 150,
  300, 100, 100, 100,
  300, 100, 100, 100,

  300, 150, 150, 300,
  150, 150, 300, 150,
  150, 300, 150, 300,
  150, 300, 300, 150,

  200, 200, 300,
  150, 200, 200,
  150, 200, 150, 200,
  200, 300
};



int doom_duration[] = {

  100, 100, 100, 100,
  100, 100, 100, 100,
  100, 100, 100, 100,
  100, 100, 100, 100,

  100, 100, 100, 100,
  100, 100, 100, 100,
  100, 100, 100, 300
};

int doom_melody[] = {

  NOTE_E2, NOTE_E2, NOTE_E3, NOTE_E2,
  NOTE_E2, NOTE_D3, NOTE_E2, NOTE_E2,
  NOTE_C3, NOTE_E2, NOTE_E2, NOTE_AS2,
  NOTE_E2, NOTE_E2, NOTE_B2, NOTE_C3,

  NOTE_E2, NOTE_E2, NOTE_E3, NOTE_E2,
  NOTE_E2, NOTE_D3, NOTE_E2, NOTE_E2,
  NOTE_C3, NOTE_E2, NOTE_E2, NOTE_AS2

};
void playDoom() {

  int tamanho = sizeof(doom_melody) / sizeof(int);

  for (int i = 0; i < tamanho; i++) {

    if (doom_melody[i] != 0) {
      tone(BUZZER_PIN, doom_melody[i]);
    }

    delay(doom_duration[i]);

    noTone(BUZZER_PIN);

    delay(20);
  }
}




RF24 radio(CE_PIN, CSN_PIN);

// ==========================================
// CONFIGURAÇÃO DO NÓ (ALTERAR PARA CADA ARDUINO)
// ==========================================
// Para o Sensor  -> Deixe ID como 44 e descomente loop_sensor() no loop()
// Para o Gateway -> Deixe ID como 1  e descomente loop_gateway() no loop()
// Para o Servidor-> Deixe ID como 20 e descomente loop_server() no loop()
#define ID 44  // ALTERAR

#define GATEWAY_ID 4
#define SERVER_ID 1
#define SENSOR_ID 44

#define DEBUG

uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL };

enum packet_type {
  RTS,
  CTS,
  DATA,
  ACK
};

struct packet {
  enum packet_type type;
  int source_ID;
  int destination_ID;
  int data;
};

struct packet received_packet;
struct packet sent_packet;


void playMario() {

  int tamanho = sizeof(mario_melody) / sizeof(int);

  for (int i = 0; i < tamanho; i++) {

    if (mario_melody[i] != 0) {
      tone(BUZZER_PIN, mario_melody[i]);
    }

    delay(duration[i]);

    noTone(BUZZER_PIN);

    delay(20);
  }
}



bool send_packet(int target_id, int data_value) {
  unsigned long timeout;

  sent_packet.type = RTS;
  sent_packet.source_ID = ID;
  sent_packet.destination_ID = target_id;
  sent_packet.data = data_value;

  // --- 1. ENVIAR RTS ---
  radio.stopListening();
  radio.flush_tx();
  if (!radio.write(&sent_packet, sizeof(struct packet))) return false;

  // --- 2. AGUARDAR CTS ---
  radio.startListening();
  radio.flush_rx();

  timeout = millis();
  while (1) {
    if (millis() - timeout > 400) {
      radio.flush_rx();
      return false;
    }

    if (radio.available()) {
      radio.read(&received_packet, sizeof(struct packet));
      // Confirma se o pacote é o CTS esperado
      if (received_packet.type == CTS && received_packet.destination_ID == ID) break;
    }
  }

#ifdef DEBUG
  Serial.println(F("-> CTS Recebido com sucesso! Enviando DATA..."));
#endif

  // --- 3. ENVIAR DATA ---
  sent_packet.type = DATA;
  radio.stopListening();
  delay(10);
  radio.flush_tx();
  if (!radio.write(&sent_packet, sizeof(struct packet))) return false;

  // --- 4. AGUARDAR ACK ---
  radio.startListening();
  radio.flush_rx();

  timeout = millis();
  while (1) {
    if (millis() - timeout > 400) {
      radio.flush_rx();
      return false;
    }

    if (radio.available()) {
      radio.read(&received_packet, sizeof(struct packet));
      if (received_packet.type == ACK && received_packet.destination_ID == ID) return true;
    }
  }

  return false;
}

// --- FUNÇÃO DE ESCUTA ---
bool await_packet() {
  unsigned long timeout;

  radio.startListening();

  if (radio.available()) {
    radio.read(&received_packet, sizeof(struct packet));

    // Valida se o RTS é direcionado para o ID deste Arduino específico
    if (received_packet.type == RTS && received_packet.destination_ID == ID) {

#ifdef DEBUG
      Serial.print(F("RTS recebido do nó: "));
      Serial.println(received_packet.source_ID);
      Serial.println(F("Aguardando 15ms de segurança antes de disparar o CTS..."));
#endif

      // --- 2. ENVIAR CTS ---
      sent_packet.type = CTS;
      sent_packet.source_ID = ID;
      sent_packet.destination_ID = received_packet.source_ID;

      radio.stopListening();

      delay(15);

      radio.flush_tx();
      radio.write(&sent_packet, sizeof(struct packet));

#ifdef DEBUG
      Serial.println(F("CTS enviado!"));
#endif

      // --- 3. AGUARDAR DATA ---
      radio.startListening();
      radio.flush_rx();
      timeout = millis();
      bool data_received = false;

      while (millis() - timeout < 500) {
        if (radio.available()) {
          radio.read(&received_packet, sizeof(struct packet));
          if (received_packet.type == DATA && received_packet.destination_ID == ID) {
            data_received = true;
            break;
          }
        }
      }

      // --- 4. ENVIAR ACK ---
      if (data_received) {
        sent_packet.type = ACK;
        sent_packet.source_ID = ID;
        sent_packet.destination_ID = received_packet.source_ID;

        radio.stopListening();
        delay(15);
        radio.flush_tx();
        radio.write(&sent_packet, sizeof(struct packet));
        return true;
      }
    }
  }
  return false;
}

// LOOP DE CADA ARDUINO

void loop_sensor() {

  leitura = digitalRead(sensor_PIN);  //leitura do sensor de presença

  if (leitura == HIGH) {  // movimento  detectado
    digitalWrite(led_PIN, HIGH);
    if (estadoSensor == false) {
      Serial.println("Movimento detectado");
      estadoSensor = true;
      Serial.println(F("Sensor: Transmitindo RTS..."));
      if (send_packet(GATEWAY_ID, digitalRead(sensor_PIN))) {
        Serial.println(F("Sensor: Transação concluída! Dados entregues."));
      } else {
        Serial.println(F("Sensor: Erro (CTS não veio ou ACK falhou)."));
      }
    }
    delay(5000);

  } else {  // sem movimento
    if (estadoSensor == true) {
      Serial.println("Sem movimento");
      estadoSensor = false;
      digitalWrite(led_PIN, LOW);
    }
  }
}

void loop_gateway() {
  if (await_packet()) {
    if (received_packet.source_ID == SENSOR_ID) {
      Serial.println(F("Gateway: Pacote recebido do Sensor."));
      int valor = received_packet.data;

      Serial.println(F("Gateway: Repassando para o Servidor..."));
      if (send_packet(SERVER_ID, valor)) {
        Serial.println(F("Gateway: Servidor respondeu ao repasse."));
      } else {
        Serial.println(F("Gateway: Servidor não respondeu."));
      }
    }
  }
}

void loop_server() {
  if (await_packet()) {
    if (received_packet.source_ID == GATEWAY_ID) {
      // Envia para o servidor python escutando Serial
      Serial.println("1");

      playMario();

      // ou
      // playDoom();
      // playDoom();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print(F("Iniciando Nó ID: "));
  Serial.println(ID);

  if (!radio.begin()) {
    Serial.println(F("ERRO: Hardware NRF24L01 ausente"));
    while (1) {}
  }

  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(100);
  radio.setPayloadSize(sizeof(struct packet));
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setDataRate(RF24_250KBPS);

  // Seus pipes intocados
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);

  radio.startListening();
  Serial.println(F("Pronto para rodar."));


  //      ATENÇÃO
  // USO PINOS PARA O SENSOR
  pinMode(led_PIN, OUTPUT);
  pinMode(sensor_PIN, INPUT);
  pinMode(BUZZER_PIN, INPUT);
}

void loop() {
  // Descomente apenas um por placa antes de compilar:

  loop_sensor();
  // loop_gateway();
  // loop_server();
}
