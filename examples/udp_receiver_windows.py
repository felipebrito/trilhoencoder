#!/usr/bin/env python3
"""
Receptor UDP para Windows - Dados do encoder ESP32
Conecte-se à rede WiFi "EncoderESP32" (senha: 12345678)
"""

import socket
import json
import time
import sys
import threading

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
        return "192.168.4.100"  # IP padrão se não conseguir detectar

def receive_encoder_data():
    # Configurações UDP
    UDP_IP = "0.0.0.0"  # Escuta em todas as interfaces
    UDP_PORT = 8888
    
    # Obtém IP local
    local_ip = get_local_ip()
    print(f"IP Local detectado: {local_ip}")
    print(f"Escutando dados UDP na porta {UDP_PORT}...")
    print("Conecte-se à rede WiFi 'EncoderESP32' para receber dados")
    print("-" * 60)
    
    # Cria socket UDP
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((UDP_IP, UDP_PORT))
        sock.settimeout(1.0)  # Timeout de 1 segundo
    except Exception as e:
        print(f"ERRO ao criar socket: {e}")
        return
    
    print("✅ Socket UDP criado com sucesso!")
    print("Aguardando dados do encoder...")
    print("-" * 60)
    
    packet_count = 0
    last_time = time.time()
    
    try:
        while True:
            try:
                # Recebe dados
                data, addr = sock.recvfrom(1024)
                packet_count += 1
                current_time = time.time()
                
                # Calcula taxa de pacotes
                if current_time - last_time >= 5.0:  # A cada 5 segundos
                    rate = packet_count / 5.0
                    print(f"📊 Taxa de pacotes: {rate:.1f} pacotes/segundo")
                    packet_count = 0
                    last_time = current_time
                
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

def test_connection():
    """Testa a conectividade de rede"""
    print("🔍 Testando conectividade...")
    
    # Testa se consegue resolver o DNS
    try:
        socket.gethostbyname("8.8.8.8")
        print("✅ Conectividade de rede OK")
    except:
        print("❌ ERRO: Sem conectividade de rede")
        return False
    
    # Testa se a porta está disponível
    try:
        test_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        test_sock.bind(("0.0.0.0", 8888))
        test_sock.close()
        print("✅ Porta 8888 disponível")
    except:
        print("❌ ERRO: Porta 8888 em uso")
        return False
    
    return True

if __name__ == "__main__":
    print("=" * 60)
    print("🔧 Receptor UDP para Encoder ESP32 - Windows")
    print("=" * 60)
    
    # Testa conectividade
    if not test_connection():
        print("❌ Falha nos testes de conectividade")
        sys.exit(1)
    
    # Inicia receptor
    receive_encoder_data() 