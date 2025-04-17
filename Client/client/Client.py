import socket
import tkinter as tk
from threading import Thread

def receive_messages(client_socket):
    while True:
        message = client_socket.recv(1024).decode('utf-8')
        message_list.insert(tk.END, message)

def send_message(event=None):
    message = my_message.get()
    my_message.set("")
    message_list.insert(tk.END, "You: " + message)
    client_socket.send(message.encode('utf-8'))

def on_closing(event=None):
    client_socket.close()
    root.destroy()

root = tk.Tk()
root.title("聊天室客户端")

message_frame = tk.Frame(root)
my_message = tk.StringVar()
my_message.set("别让网友感到寂寞")

scrollbar = tk.Scrollbar(message_frame)

message_list = tk.Listbox(message_frame, height=15, width=50, yscrollcommand=scrollbar.set)
scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
message_list.pack(side=tk.LEFT, fill=tk.BOTH)
message_list.pack()

message_frame.pack()

entry_field = tk.Entry(root, textvariable=my_message)
entry_field.bind("<Return>", send_message)
entry_field.pack()

send_button = tk.Button(root, text="Send", command=send_message)
send_button.pack()

root.protocol("WM_DELETE_WINDOW", on_closing)

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('localhost', 12345))

receive_thread = Thread(target=receive_messages, args=(client_socket,))
receive_thread.start()

tk.mainloop()
