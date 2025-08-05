#!/usr/bin/env python3
"""
Exemplo de receptor UDP para dados do encoder ESP32
Conecte-se à rede WiFi "EncoderESP32" (senha: 12345678)
"""

import socket
import json
import time

def receive_encoder_data():
    # Configurações UDP
    UDP_IP = "0.0.0.0"  # Escuta em todas as interfaces
    UDP_PORT = 8888
    
    # Cria socket UDP
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    
    print(f"Escutando dados UDP na porta {UDP_PORT}...")
    print("Conecte-se à rede WiFi 'EncoderESP32' para receber dados")
    print("-" * 50)
    
    try:
        while True:
            # Recebe dados
            data, addr = sock.recvfrom(1024)
            
            try:
                # Decodifica JSON
                json_data = json.loads(data.decode('utf-8'))
                encoder_data = json_data['encoder']
                
                # Exibe dados
                print(f"Encoder: {encoder_data['pulses']} pulsos | "
                      f"Distância: {encoder_data['distance']} cm | "
                      f"Timestamp: {encoder_data['timestamp']} ms")
                
            except json.JSONDecodeError:
                print(f"Dados recebidos (não JSON): {data.decode('utf-8')}")
            except KeyError:
                print(f"Formato JSON inválido: {json_data}")
                
    except KeyboardInterrupt:
        print("\nReceptor UDP encerrado")
    finally:
        sock.close()

if __name__ == "__main__":
    receive_encoder_data() 