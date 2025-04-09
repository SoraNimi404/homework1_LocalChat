import os
import sys
import socket
import daemon


# 守护进程的设置函数
def run_server():
    host = '0.0.0.0'
    port = 12345
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((host, port))
    server_socket.listen(5)

    print("Server is running...")

    try:
        while True:
            client_socket, addr = server_socket.accept()
            print(f"Connected by {addr}")
            while True:
                data = client_socket.recv(1024)
                if not data:
                    break
                print(f"Received from {addr}: {data.decode()}")
                client_socket.sendall(data)
            client_socket.close()
    except KeyboardInterrupt:
        print("Server is shutting down...")
    finally:
        server_socket.close()


# 运行守护进程
def run_daemon():
    with daemon.DaemonContext():
        run_server()


if __name__ == "__main__":
    run_daemon()
