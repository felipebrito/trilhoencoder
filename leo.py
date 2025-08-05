#!/usr/bin/env python3
"""
Receptor UDP simples para dados do encoder ESP32
Conecte-se à rede WiFi "EncoderESP32" (senha: 12345678)
"""

import socket
import json
import time

def get_local_ip():
    """Obtém o IP local da máquina"""
    try:
        # Cria um socket temporário para obter o IP local
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
        return local_ip
    except:
        return "192.168.4.2"  # IP padrão se não conseguir detectar

def receive_encoder_data():
    # Configurações UDP
    UDP_IP = "0.0.0.0"  # Escuta em todas as interfaces
    UDP_PORT = 8888
    
    # Obtém IP local
    local_ip = get_local_ip()
    
    print("=" * 50)
    print("🔧 Receptor UDP para Encoder ESP32")
    print("=" * 50)
    print(f"IP Local detectado: {local_ip}")
    print(f"Escutando dados UDP na porta {UDP_PORT}...")
    print("Conecte-se à rede WiFi 'EncoderESP32' para receber dados")
    print("-" * 50)
    
    # Cria socket UDP
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((UDP_IP, UDP_PORT))
        sock.settimeout(1.0)  # Timeout de 1 segundo
        print("✅ Socket UDP criado com sucesso!")
    except Exception as e:
        print(f"❌ ERRO ao criar socket: {e}")
        return
    
    print("Aguardando dados do encoder...")
    print("-" * 50)
    
    try:
        while True:
            try:
                # Recebe dados
                data, addr = sock.recvfrom(1024)
                
                try:
                    # Decodifica JSON
                    json_data = json.loads(data.decode('utf-8'))
                    encoder_data = json_data['encoder']
                    
                    # Exibe dados formatados
                    print(f"📡 {addr[0]}:{addr[1]} | "
                          f"Encoder: {encoder_data['pulses']:6d} pulsos | "
                          f"Distância: {encoder_data['distance']:6.2f} cm | "
                          f"Timestamp: {encoder_data['timestamp']:8d} ms")
                    
                except json.JSONDecodeError as e:
                    print(f"❌ ERRO JSON de {addr[0]}:{addr[1]}: {e}")
                    print(f"   Dados recebidos: {data.decode('utf-8', errors='ignore')}")
                except KeyError as e:
                    print(f"❌ ERRO formato JSON de {addr[0]}:{addr[1]}: {e}")
                    print(f"   JSON recebido: {json_data}")
                    
            except socket.timeout:
                # Timeout - continua o loop
                continue
            except Exception as e:
                print(f"❌ ERRO ao receber dados: {e}")
                break
                
    except KeyboardInterrupt:
        print("\n🛑 Receptor UDP encerrado pelo usuário")
    except Exception as e:
        print(f"❌ ERRO fatal: {e}")
    finally:
        sock.close()
        print("✅ Socket UDP fechado")

if __name__ == "__main__":
    receive_encoder_data() 