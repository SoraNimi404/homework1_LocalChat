import tkinter as tk
from tkinter import simpledialog, messagebox
import socket
import threading
import os

# 默认服务器地址和端口
DEFAULT_SERVER_ADDRESS = 'localhost'
DEFAULT_PORT = 12345

def ask_for_port(default_address, default_port):
    root = tk.Tk()
    root.withdraw()  # 隐藏主窗口
    # 弹出对话框让用户输入端口，默认值为默认端口
    port = simpledialog.askinteger(
        "输入端口",
        f"请输入你想要连接的端口\n服务器地址: {default_address}\n默认端口: {default_port}",
        minvalue=1, maxvalue=65535, initialvalue=default_port
    )
    root.destroy()
    return port

def connect_to_server(server_address, port):
    try:
        client_socket.connect((server_address, port))
        return True
    except socket.error as e:
        messagebox.showerror("连接错误", f"无法连接到服务器: {e}")
        return False

# 创建主窗口
root = tk.Tk()
root.title("聊天客户端")

# 增加窗口宽度
window_width = 550
window_height = 600

# 设置窗口大小
root.geometry(f"{window_width}x{window_height}")

# 让用户选择端口
port = ask_for_port(DEFAULT_SERVER_ADDRESS, DEFAULT_PORT)
if port is None:
    os.exit(0)  # 用户取消输入，退出程序

# 创建客户端套接字
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# 尝试连接到服务器
if not connect_to_server(DEFAULT_SERVER_ADDRESS, port):
    os.exit(1)  # 连接失败，退出程序

# 创建消息显示列表
message_listbox = tk.Listbox(root, height=15, width=50)
message_listbox.pack()

# 创建消息输入框
message_entry = tk.Entry(root, width=50)
message_entry.pack()

# 绑定发送消息事件
def send_message(event=None):
    # 发送消息的代码 here
    pass

message_entry.bind("<Return>", send_message)

# 创建发送按钮
send_button = tk.Button(root, text="发送", command=send_message)
send_button.pack()

# 启动接收消息的线程
def receive_messages():
    # 接收消息的代码 here
    pass

receive_thread = threading.Thread(target=receive_messages)
receive_thread.start()

# 设置窗口关闭事件
def on_closing():
    if messagebox.askokcancel("退出", "确定要退出吗?"):
        client_socket.close()
        root.destroy()

root.protocol("WM_DELETE_WINDOW", on_closing)

# 运行主循环
root.mainloop()
