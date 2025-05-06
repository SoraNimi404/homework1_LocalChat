import socket
import threading
import tkinter as tk
from tkinter import scrolledtext, messagebox

class ChatClient:
    def __init__(self, master, host='localhost', port=12345):
        self.master = master
        self.host = host
        self.port = port
        self.nickname = "匿名"
        self.socket = None
        self.connected = False
        
        self.setup_ui()
        
    def setup_ui(self):
        self.master.title("聊天室客户端")
        self.master.geometry("500x400")
        
        # 连接设置框架
        self.conn_frame = tk.Frame(self.master)
        self.conn_frame.pack(pady=10)
        
        tk.Label(self.conn_frame, text="服务器地址:").grid(row=0, column=0)
        self.host_entry = tk.Entry(self.conn_frame)
        self.host_entry.insert(0, self.host)
        self.host_entry.grid(row=0, column=1)
        
        tk.Label(self.conn_frame, text="端口:").grid(row=0, column=2)
        self.port_entry = tk.Entry(self.conn_frame, width=6)
        self.port_entry.insert(0, str(self.port))
        self.port_entry.grid(row=0, column=3)
        
        tk.Label(self.conn_frame, text="昵称:").grid(row=1, column=0)
        self.nick_entry = tk.Entry(self.conn_frame)
        self.nick_entry.insert(0, self.nickname)
        self.nick_entry.grid(row=1, column=1)
        
        self.connect_btn = tk.Button(
            self.conn_frame, text="连接", command=self.toggle_connection)
        self.connect_btn.grid(row=1, column=2, columnspan=2, padx=5)
        
        # 聊天显示区域
        self.chat_area = scrolledtext.ScrolledText(
            self.master, wrap=tk.WORD, state='disabled')
        self.chat_area.pack(padx=10, pady=5, fill=tk.BOTH, expand=True)
        
        # 消息发送区域
        self.msg_frame = tk.Frame(self.master)
        self.msg_frame.pack(pady=10, fill=tk.X)
        
        self.msg_entry = tk.Entry(self.msg_frame)
        self.msg_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=5)
        self.msg_entry.bind("<Return>", self.send_message)
        
        self.send_btn = tk.Button(
            self.msg_frame, text="发送", command=self.send_message)
        self.send_btn.pack(side=tk.RIGHT, padx=5)
        
    def toggle_connection(self):
        if self.connected:
            self.disconnect()
        else:
            self.connect()
    
    def connect(self):
        try:
            self.host = self.host_entry.get()
            self.port = int(self.port_entry.get())
            self.nickname = self.nick_entry.get().strip() or "匿名"
            
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            
            # 发送昵称
            self.socket.send(f"/nick {self.nickname}".encode('utf-8'))
            
            self.connected = True
            self.connect_btn.config(text="断开")
            self.host_entry.config(state='disabled')
            self.port_entry.config(state='disabled')
            self.nick_entry.config(state='disabled')
            
            # 启动接收线程
            receive_thread = threading.Thread(target=self.receive_messages)
            receive_thread.daemon = True
            receive_thread.start()
            
            self.display_message("系统", f"已连接到 {self.host}:{self.port}")
        except Exception as e:
            messagebox.showerror("连接错误", f"无法连接到服务器: {str(e)}")
    
    def disconnect(self):
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
        self.connected = False
        self.connect_btn.config(text="连接")
        self.host_entry.config(state='normal')
        self.port_entry.config(state='normal')
        self.nick_entry.config(state='normal')
        self.display_message("系统", "已断开与服务器的连接")
    
    def receive_messages(self):
        while self.connected:
            try:
                message = self.socket.recv(1024).decode('utf-8')
                if not message:
                    self.disconnect()
                    break
                
                # 解析消息 (格式: "昵称: 消息内容")
                if ": " in message:
                    sender, content = message.split(": ", 1)
                    self.display_message(sender, content)
                else:
                    self.display_message("系统", message)
            except:
                self.disconnect()
                break
    
    def send_message(self, event=None):
        if not self.connected:
            messagebox.showwarning("未连接", "请先连接到服务器")
            return
            
        message = self.msg_entry.get().strip()
        if message:
            try:
                self.socket.send(message.encode('utf-8'))
                self.display_message(self.nickname, message)
                self.msg_entry.delete(0, tk.END)
            except:
                self.disconnect()
                messagebox.showerror("错误", "发送消息失败，连接已断开")
    
    def display_message(self, sender, message):
        self.chat_area.config(state='normal')
        self.chat_area.insert(tk.END, f"{sender}: {message}\n")
        self.chat_area.config(state='disabled')
        self.chat_area.see(tk.END)
    
    def on_closing(self):
        self.disconnect()
        self.master.destroy()

if __name__ == "__main__":
    print("程序开始执行")  # 检查是否执行到这里
    root = tk.Tk()
    print("Tk() 初始化完成")  # 检查是否执行到这里
    client = ChatClient(root)
    print("ChatClient 初始化完成")  # 检查是否执行到这里
    root.protocol("WM_DELETE_WINDOW", client.on_closing)
    root.mainloop()
    print("进入主循环") 