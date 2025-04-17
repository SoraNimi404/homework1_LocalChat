import socket
import os
import sys
import threading

def daemonize():
    # 创建子进程，脱离父进程
    if os.fork() > 0:
        sys.exit()
    # 创建新会话，脱离终端
    os.setsid()
    # 再次创建子进程，确保不会成为会话首进程
    if os.fork() > 0:
        sys.exit()
    # 重定向标准文件描述符
    sys.stdout.flush()
    sys.stderr.flush()
    with open('/dev/null', 'r+b') as f:
        os.dup2(f.fileno(), sys.stdin.fileno())
        os.dup2(f.fileno(), sys.stdout.fileno())
        os.dup2(f.fileno(), sys.stderr.fileno())

def handle_client(client_socket, client_address, clients):
    try:
        while True:
            message = client_socket.recv(1024).decode('utf-8')
            if not message:
                break
            for c in clients:
                if c != client_socket:
                    c.send(message.encode('utf-8'))
    finally:
        client_socket.close()
        clients.remove(client_socket)
        print(f"Client {client_address} disconnected.")

def main():
    daemonize()
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', 12345))
    server_socket.listen(5)
    clients = []

    try:
        while True:
            client_socket, client_address = server_socket.accept()
            print(f"New connection from {client_address}")
            clients.append(client_socket)
            client_thread = threading.Thread(target=handle_client, args=(client_socket, client_address, clients))
            client_thread.start()
    finally:
        server_socket.close()

if __name__ == '__main__':
    main()
