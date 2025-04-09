import socket
import tkinter as tk
from tkinter import simpledialog

def send_message():
    message = entry.get()
    if message:
        client_socket.sendall(message.encode())
        entry.delete(0, tk.END)

def receive_messages():
    while True:
        try:
            data = client_socket.recv(1024)
            if data:
                text_display.insert(tk.END, data.decode() + '\n')
        except:
            break

def connect_to_server():
    global client_socket
    server_ip = simpledialog.askstring("Server IP", "Enter the server IP address:")
    if server_ip:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((server_ip, 12345))
        receive_thread.start()

client_socket = None
receive_thread = None

root = tk.Tk()
root.title("Chat Client")

text_display = tk.Text(root, height=20, width=50)
text_display.pack()

entry = tk.Entry(root, width=40)
entry.pack(side=tk.LEFT, padx=(5,0))

send_button = tk.Button(root, text="Send", command=send_message)
send_button.pack(side=tk.RIGHT, padx=(0,5))

connect_button = tk.Button(root, text="Connect", command=connect_to_server)
connect_button.pack(side=tk.BOTTOM, pady=(5,0))

receive_thread = threading.Thread(target=receive_messages)

root.mainloop()
