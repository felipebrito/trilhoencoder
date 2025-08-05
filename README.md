# Encoder ESP32 - Sistema de Medição

Sistema simples de medição de distância usando encoder rotativo com interface web e transmissão UDP.

## Características

- **Encoder rotativo**: Leitura precisa de posição
- **Interface web**: Acesso via browser
- **WiFi AP**: Rede própria para conexão
- **Medição em tempo real**: Dados atualizados automaticamente
- **Transmissão UDP**: Broadcast de dados para outros dispositivos

## Hardware

- ESP32
- Encoder rotativo conectado aos pinos 18 e 19
- 1 rotação = 20 cm, 4000 pulsos por rotação

## Instalação

1. Clone este repositório
2. Abra o projeto no PlatformIO
3. Compile e carregue no ESP32

## Uso

1. **Carregue o código** no ESP32
2. **Conecte-se** à rede WiFi "EncoderESP32" (senha: 12345678)
3. **Acesse** o IP mostrado no Serial Monitor (geralmente 192.168.4.1)
4. **Veja os dados** em tempo real no browser

## Transmissão UDP

O sistema envia dados via UDP broadcast a cada 100ms:

- **IP**: 192.168.4.255 (broadcast)
- **Porta**: 8888
- **Formato**: JSON
- **Frequência**: 100ms

### Exemplo de dados UDP:
```json
{
  "encoder": {
    "pulses": 1234,
    "distance": 6.17,
    "timestamp": 12345
  }
}
```

### Receptor UDP (Python):
```bash
python3 examples/udp_receiver_example.py
```

### Receptor UDP para Windows:
```bash
python3 examples/udp_receiver_windows.py
```

## Troubleshooting UDP - Windows

Se o UDP não funcionar no Windows:

1. **Conecte-se** à rede WiFi "EncoderESP32"
2. **Verifique seu IP**: O receptor detectará automaticamente
3. **Use o receptor Windows**: `python3 examples/udp_receiver_windows.py`
4. **Alternar modo UDP**: Use o botão "UDP" na interface web
   - **IP ESPECÍFICO**: Envia para 192.168.4.100 (padrão)
   - **BROADCAST**: Envia para todos os dispositivos
5. **Configure seu IP**: Se necessário, altere `udpTargetIP` no código

### Verificações:
- ✅ Conectividade de rede
- ✅ Porta 8888 disponível
- ✅ Firewall do Windows (desabilite temporariamente)
- ✅ Antivírus (pode bloquear UDP)

## Arquivos

- `examples/encoder_ap.cpp` - Código principal com interface web e UDP
- `examples/udp_receiver_example.py` - Exemplo de receptor UDP
- `platformio.ini` - Configuração do PlatformIO
- `README.md` - Este arquivo

## Funcionalidades

- **Medição de distância** em centímetros
- **Contador de pulsos** do encoder
- **Reset do encoder** via interface web
- **Atualização automática** a cada 100ms
- **Interface responsiva** para desktop e mobile
- **Transmissão UDP** para outros dispositivos

## Conexões

- **Pino 18**: Sinal A do encoder
- **Pino 19**: Sinal B do encoder
- **GND**: Terra comum
- **3.3V**: Alimentação do encoder

## Configuração

O sistema cria um Access Point WiFi:
- **SSID**: EncoderESP32
- **Senha**: 12345678
- **IP**: 192.168.4.1
- **UDP Broadcast**: 192.168.4.255:8888

## Desenvolvimento

Para modificar o código:
1. Edite `examples/encoder_ap.cpp`
2. Compile com PlatformIO
3. Carregue no ESP32

## Licença

Este projeto é de código aberto. 