#include <WiFi.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <ESP32Encoder.h>

ESP32Encoder encoder;

const float cmPorPulso = 20.0 / 4000.0;  // 1 rotação = 20 cm, 4000 pulsos por rotação

// Configurações de rede
const char* ssid = "EncoderESP32";
const char* password = "12345678";
IPAddress localIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Configurações UDP
WiFiUDP udp;
const int udpPort = 8888;
// Configurações para Windows - usar IP específico em vez de broadcast
const char* udpTargetIP = "192.168.4.2"; // IP específico para teste
const char* broadcastIP = "192.168.4.255"; // Broadcast para todos os dispositivos na rede
bool useBroadcast = false; // Flag para alternar entre broadcast e IP específico

// Servidor web
WebServer server(80);

// Função para servir a página HTML
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Encoder ESP32 - Medição</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #1a1a1a;
            color: #ffffff;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: #2d2d2d;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.3);
        }
        .status {
            background: #3a3a3a;
            padding: 15px;
            border-radius: 5px;
            margin: 10px 0;
            border-left: 4px solid #4CAF50;
        }
        .button {
            background: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            margin: 5px;
            font-size: 14px;
            transition: all 0.3s ease;
        }
        .button:hover {
            background: #45a049;
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.4);
        }
        .button:active {
            transform: translateY(0);
            box-shadow: 0 2px 4px rgba(0,0,0,0.4);
        }
        .button.reset {
            background: #f44336;
        }
        .button.reset:hover {
            background: #da190b;
        }
        .button.udp {
            background: #9C27B0;
        }
        .button.udp:hover {
            background: #7B1FA2;
        }
        .button.test {
            background: #FF9800;
        }
        .button.test:hover {
            background: #F57C00;
        }
        .data {
            font-family: monospace;
            font-size: 18px;
            margin: 10px 0;
            text-align: center;
        }
        .value {
            font-size: 24px;
            font-weight: bold;
            color: #2196F3;
        }
        .button-container {
            display: flex;
            justify-content: center;
            gap: 10px;
            flex-wrap: wrap;
        }
        .status-message {
            background: #2d5a2d;
            color: #4CAF50;
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
            text-align: center;
            display: none;
        }
        h1 {
            color: #ffffff;
            text-align: center;
            margin-bottom: 30px;
        }
        h3 {
            color: #e0e0e0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Encoder ESP32 - Sistema de Medição</h1>
        
        <div class="status">
            <h3>Dados do Encoder</h3>
            <div class="data">
                <div>Distância: <span class="value" id="distance">--</span> cm</div>
                <div>Pulsos: <span id="pulses">--</span></div>
            </div>
        </div>
        
        <div class="status">
            <h3>Controles</h3>
            <div class="button-container">
                <button class="button reset" onclick="resetEncoder()">Resetar Encoder</button>
                <button class="button udp" onclick="toggleUDP()">UDP: <span id="udpMode">IP ESPECÍFICO</span></button>
                <button class="button test" onclick="testUDP()">Testar UDP</button>
            </div>
            <div id="statusMessage" class="status-message"></div>
        </div>
    </div>

    <script>
        let lastUpdate = 0;
        const UPDATE_INTERVAL = 100; // 100ms igual ao serial
        
        function updateData() {
            const now = Date.now();
            if (now - lastUpdate < UPDATE_INTERVAL) return; // Evita atualizações muito frequentes
            
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('distance').textContent = data.distance.toFixed(2);
                    document.getElementById('pulses').textContent = data.pulses;
                    lastUpdate = now;
                })
                .catch(error => {
                    console.error('Erro ao buscar dados:', error);
                });
        }
        
        function resetEncoder() {
            const resetButton = document.querySelector('.button.reset');
            const statusMessage = document.getElementById('statusMessage');
            
            // Desabilita o botão durante o reset
            resetButton.disabled = true;
            resetButton.textContent = 'Resetando...';
            
            fetch('/reset')
                .then(response => response.text())
                .then(result => {
                    // Mostra mensagem de sucesso
                    statusMessage.textContent = '✅ Encoder resetado com sucesso!';
                    statusMessage.style.display = 'block';
                    statusMessage.style.background = '#2d5a2d';
                    statusMessage.style.color = '#4CAF50';
                    
                    // Atualiza os dados imediatamente
                    updateData();
                    
                    // Esconde a mensagem após 3 segundos
                    setTimeout(() => {
                        statusMessage.style.display = 'none';
                    }, 3000);
                })
                .catch(error => {
                    console.error('Erro ao resetar:', error);
                    
                    // Mostra mensagem de erro
                    statusMessage.textContent = '❌ Erro ao resetar o encoder!';
                    statusMessage.style.display = 'block';
                    statusMessage.style.background = '#5a2d2d';
                    statusMessage.style.color = '#f44336';
                    
                    // Esconde a mensagem após 3 segundos
                    setTimeout(() => {
                        statusMessage.style.display = 'none';
                    }, 3000);
                })
                .finally(() => {
                    // Reabilita o botão
                    resetButton.disabled = false;
                    resetButton.textContent = 'Resetar Encoder';
                });
        }
        
        function toggleUDP() {
            const udpButton = document.querySelector('.button.udp');
            const udpModeSpan = document.getElementById('udpMode');
            const statusMessage = document.getElementById('statusMessage');
            
            fetch('/udp_toggle')
                .then(response => response.json())
                .then(data => {
                    udpModeSpan.textContent = data.mode;
                    statusMessage.textContent = `✅ UDP alterado para: ${data.mode} (${data.target})`;
                    statusMessage.style.display = 'block';
                    statusMessage.style.background = '#2d5a2d';
                    statusMessage.style.color = '#4CAF50';
                    
                    setTimeout(() => {
                        statusMessage.style.display = 'none';
                    }, 3000);
                })
                .catch(error => {
                    console.error('Erro ao alternar UDP:', error);
                    statusMessage.textContent = '❌ Erro ao alternar modo UDP!';
                    statusMessage.style.display = 'block';
                    statusMessage.style.background = '#5a2d2d';
                    statusMessage.style.color = '#f44336';
                    
                    setTimeout(() => {
                        statusMessage.style.display = 'none';
                    }, 3000);
                });
        }
        
        function testUDP() {
            const testButton = document.querySelector('.button.test');
            const statusMessage = document.getElementById('statusMessage');
            
            testButton.disabled = true;
            testButton.textContent = 'Testando...';
            
            fetch('/udp_test')
                .then(response => response.text())
                .then(result => {
                    statusMessage.textContent = '✅ Teste UDP executado! Verifique o Serial Monitor';
                    statusMessage.style.display = 'block';
                    statusMessage.style.background = '#2d5a2d';
                    statusMessage.style.color = '#4CAF50';
                    
                    setTimeout(() => {
                        statusMessage.style.display = 'none';
                    }, 3000);
                })
                .catch(error => {
                    console.error('Erro ao testar UDP:', error);
                    statusMessage.textContent = '❌ Erro ao testar UDP!';
                    statusMessage.style.display = 'block';
                    statusMessage.style.background = '#5a2d2d';
                    statusMessage.style.color = '#f44336';
                    
                    setTimeout(() => {
                        statusMessage.style.display = 'none';
                    }, 3000);
                })
                .finally(() => {
                    testButton.disabled = false;
                    testButton.textContent = 'Testar UDP';
                });
        }
        
        // Atualiza dados a cada 100ms (igual ao serial)
        setInterval(updateData, 100);
        
        // Atualiza dados quando a página carrega
        window.onload = function() {
            updateData();
        };
    </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

// Função para retornar dados do encoder em JSON
void handleData() {
  long pulsos = encoder.getCount();
  float distancia_cm = pulsos * cmPorPulso;
  
  String json = "{\"distance\":" + String(distancia_cm, 2) + ",\"pulses\":" + String(pulsos) + "}";
  
  // Headers para melhor performance
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// Função para resetar o encoder
void handleReset() {
  encoder.setCount(0);
  server.send(200, "text/plain", "Encoder resetado");
}

// Função para alternar modo UDP
void handleUDPToggle() {
  useBroadcast = !useBroadcast;
  String mode = useBroadcast ? "BROADCAST" : "IP ESPECÍFICO";
  String target = useBroadcast ? broadcastIP : udpTargetIP;
  
  Serial.print("UDP alterado para: "); Serial.print(mode); Serial.print(" - "); Serial.println(target);
  
  String response = "{\"mode\":\"" + mode + "\",\"target\":\"" + target + "\"}";
  server.send(200, "application/json", response);
}

// Função para testar UDP manualmente
void handleUDPTest() {
  Serial.println("=== TESTE UDP MANUAL ===");
  Serial.print("Dispositivos conectados: "); Serial.println(WiFi.softAPgetStationNum());
  Serial.print("IP do AP: "); Serial.println(WiFi.softAPIP());
  Serial.print("Modo UDP: "); Serial.println(useBroadcast ? "BROADCAST" : "IP ESPECÍFICO");
  
  // Força envio de dados UDP
  sendUDPData();
  
  server.send(200, "text/plain", "Teste UDP executado - verifique o Serial Monitor");
}

// Função para enviar dados via UDP
void sendUDPData() {
  long pulsos = encoder.getCount();
  float distancia_cm = pulsos * cmPorPulso;
  
  // Cria string JSON com os dados
  String jsonData = "{\"encoder\":{\"pulses\":" + String(pulsos) + ",\"distance\":" + String(distancia_cm, 2) + ",\"timestamp\":" + String(millis()) + "}}";
  
  // Verifica se o WiFi está ativo
  if (WiFi.softAPgetStationNum() == 0) {
    Serial.println("AVISO: Nenhum dispositivo conectado ao AP");
    return;
  }
  
  // Tenta enviar para IP específico primeiro (melhor para Windows)
  if (!useBroadcast) {
    Serial.print("Tentando enviar UDP para "); Serial.print(udpTargetIP); Serial.print(":"); Serial.println(udpPort);
    
    if (udp.beginPacket(udpTargetIP, udpPort)) {
      udp.write((uint8_t*)jsonData.c_str(), jsonData.length());
      bool sent = udp.endPacket();
      
      if (sent) {
        Serial.print("✅ UDP enviado para "); Serial.print(udpTargetIP); Serial.print(":"); Serial.print(udpPort); Serial.print(" - "); Serial.println(jsonData);
      } else {
        Serial.println("❌ ERRO: Falha ao enviar UDP para IP específico");
      }
    } else {
      Serial.println("❌ ERRO: Falha ao iniciar pacote UDP");
    }
  } else {
    // Tenta broadcast como fallback
    Serial.print("Tentando enviar UDP broadcast para "); Serial.print(broadcastIP); Serial.print(":"); Serial.println(udpPort);
    
    if (udp.beginPacket(broadcastIP, udpPort)) {
      udp.write((uint8_t*)jsonData.c_str(), jsonData.length());
      bool sent = udp.endPacket();
      
      if (sent) {
        Serial.print("✅ UDP broadcast enviado para "); Serial.print(broadcastIP); Serial.print(":"); Serial.print(udpPort); Serial.print(" - "); Serial.println(jsonData);
      } else {
        Serial.println("❌ ERRO: Falha ao enviar UDP broadcast");
      }
    } else {
      Serial.println("❌ ERRO: Falha ao iniciar pacote UDP broadcast");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Conecte os canais A e B do encoder aos pinos 18 e 19
  encoder.attachFullQuad(18, 19);
  encoder.setCount(0);  // Zera a contagem
  
  // Configuração do WiFi em modo AP
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(localIP, gateway, subnet);
  
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("Password: "); Serial.println(password);
  
  // Configuração do servidor web
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/reset", handleReset);
  server.on("/udp_toggle", handleUDPToggle);
  server.on("/udp_test", handleUDPTest);
  server.begin();
  
  Serial.println("=== SISTEMA PRONTO ===");
  Serial.println("Conecte-se à rede WiFi e acesse o IP mostrado acima");
  Serial.print("UDP Broadcast: "); Serial.print(broadcastIP); Serial.print(":"); Serial.println(udpPort);
  Serial.print("UDP IP Específico: "); Serial.print(udpTargetIP); Serial.print(":"); Serial.println(udpPort);
  Serial.print("Modo UDP inicial: "); Serial.println(useBroadcast ? "BROADCAST" : "IP ESPECÍFICO");
  Serial.println("Dados enviados via UDP a cada 100ms");
  Serial.println("Use o botão UDP na interface web para alternar o modo");
}

void loop() {
  // Atende requisições web com mais frequência
  server.handleClient();
  
  // LÓGICA ORIGINAL DO USUÁRIO - RESPETADA COMPLETAMENTE
  static unsigned long lastSerialTime = 0;
  if (millis() - lastSerialTime >= 100) {
    long pulsos = encoder.getCount();
    float distancia_cm = pulsos * cmPorPulso;
    Serial.println(distancia_cm);
    
    // Envia dados via UDP
    sendUDPData();
    
    lastSerialTime = millis();
  }
} 