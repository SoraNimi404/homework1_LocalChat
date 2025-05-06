import socket
import select
import sys
import os
import time
from daemonize import Daemonize

class ChatServer:
    def __init__(self, host='0.0.0.0', port=12345):
        self.host = host
        self.port = port
        self.server_socket = None
        self.client_sockets = []
        self.nicknames = {}
        
    def start(self):
        # ����������socket
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        
        print(f"������������������ {self.host}:{self.port}")
        
        while True:
            try:
                # ʹ��select���������
                read_sockets, _, exception_sockets = select.select(
                    [self.server_socket] + self.client_sockets, [], [])
                
                for notified_socket in read_sockets:
                    # ������
                    if notified_socket == self.server_socket:
                        client_socket, client_address = self.server_socket.accept()
                        self.client_sockets.append(client_socket)
                        print(f"���������� {client_address}")
                    # �������ӵ���Ϣ
                    else:
                        try:
                            message = notified_socket.recv(1024).decode('utf-8')
                            if not message:
                                self.remove_client(notified_socket)
                                continue
                                
                            # ������Ϣ
                            self.handle_message(notified_socket, message)
                        except:
                            self.remove_client(notified_socket)
                            continue
                
                # �����쳣socket
                for notified_socket in exception_sockets:
                    self.remove_client(notified_socket)
                    
            except KeyboardInterrupt:
                print("�������ر�")
                for client_socket in self.client_sockets:
                    client_socket.close()
                self.server_socket.close()
                sys.exit()
    
    def handle_message(self, sender_socket, message):
        # �����ǳ�����
        if message.startswith("/nick "):
            nickname = message[6:].strip()
            self.nicknames[sender_socket] = nickname
            reply = f"ϵͳ: ����ǳ�������Ϊ {nickname}"
            sender_socket.send(reply.encode('utf-8'))
            return
            
        # �㲥��Ϣ
        sender_nick = self.nicknames.get(sender_socket, "����")
        broadcast_msg = f"{sender_nick}: {message}"
        
        for client_socket in self.client_sockets:
            if client_socket != sender_socket:
                try:
                    client_socket.send(broadcast_msg.encode('utf-8'))
                except:
                    self.remove_client(client_socket)
    
    def remove_client(self, client_socket):
        if client_socket in self.client_sockets:
            self.client_sockets.remove(client_socket)
        if client_socket in self.nicknames:
            nickname = self.nicknames.pop(client_socket)
            print(f"{nickname} �ѶϿ�����")
        client_socket.close()

def main():
    server = ChatServer()
    server.start()

if __name__ == "__main__":
    pid_file = '/tmp/chat_server.pid'
    daemon = Daemonize(app="chat_server", pid=pid_file, action=main)
    daemon.start()