import socket
import threading
import logging

# 设置日志记录
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# 客户端列表
clients = []

def handle_client(client_socket, client_address, clients):
    try:
        while True:
            message = client_socket.recv(1024).decode('utf-8')
            if not message:
                break
            for c in clients:
                if c != client_socket:
                    c.send(message.encode('utf-8'))
    except Exception as e:
        logging.error(f"Error handling client {client_address}: {e}")
    finally:
        client_socket.close()
        clients.remove(client_socket)
        logging.info(f"Client {client_address} disconnected.")

def main():
    # daemonize() # 取消注释以在后台运行
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('0.0.0.0', 10000)  # 服务器地址和端口
    server_socket.bind(server_address)
    server_socket.listen(5)
    logging.info("Server is running and listening for connections...")

    try:
        while True:
            client_socket, client_address = server_socket.accept()
            logging.info(f"New connection from {client_address}")
            clients.append(client_socket)
            client_thread = threading.Thread(target=handle_client, args=(client_socket, client_address, clients))
            client_thread.start()
    finally:
        server_socket.close()

if __name__ == '__main__':
    main()
