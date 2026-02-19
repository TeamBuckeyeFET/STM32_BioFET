
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import serial
import serial.tools.list_ports
import threading
import time
import csv

class BioFETGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("STM32 BioFET Controller")
        self.root.geometry("600x500")
        
        self.serial_port = None
        self.is_connected = False
        self.custom_receive_file = ""  # Initialize as empty string instead of None
        self.data_capture_mode = False
        self.captured_data = []
        self.listen_thread = None # Initialize to None

        # --- STYLE ---
        self.style = ttk.Style()
        self.style.theme_use('clam')
        
        # --- CONNECTION FRAME ---
        conn_frame = ttk.LabelFrame(root, text="Connection", padding=10)
        conn_frame.pack(fill="x", padx=10, pady=5)
        
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(conn_frame, textvariable=self.port_var)
        self.port_combo.pack(side="left", padx=5)
        self.refresh_ports()
        
        self.btn_refresh = ttk.Button(conn_frame, text="Refresh", command=self.refresh_ports)
        self.btn_refresh.pack(side="left", padx=5)
        
        self.btn_connect = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.btn_connect.pack(side="left", padx=5)
        
        # --- TEST CONFIGURATION FRAME ---
        config_frame = ttk.LabelFrame(root, text="Configuration", padding=10)
        config_frame.pack(fill="x", padx=10, pady=5)
        
        # Test Type
        ttk.Label(config_frame, text="Test Type:").grid(row=0, column=0, sticky="w", pady=5)
        self.test_type_var = tk.IntVar(value=2)
        ttk.Radiobutton(config_frame, text="Type 1 (Constant)", variable=self.test_type_var, value=1, command=self.update_ui_state).grid(row=0, column=1, sticky="w")
        ttk.Radiobutton(config_frame, text="Type 2 (Ramping)", variable=self.test_type_var, value=2, command=self.update_ui_state).grid(row=0, column=2, sticky="w")
        
        # Test Length
        ttk.Label(config_frame, text="Test Length (min):").grid(row=1, column=0, sticky="w", pady=5)
        self.length_var = tk.DoubleVar(value=5.0)
        self.entry_length = ttk.Entry(config_frame, textvariable=self.length_var)
        self.entry_length.grid(row=1, column=1, columnspan=2, sticky="ew")
        
        # Save Settings Button
        self.btn_save_settings = ttk.Button(config_frame, text="SAVE SETTINGS TO DEVICE", command=self.save_settings, state="disabled")
        self.btn_save_settings.grid(row=2, column=0, columnspan=3, sticky="ew", pady=10)
        
        # --- CONTROLS FRAME ---
        ctrl_frame = ttk.Frame(root, padding=10)
        ctrl_frame.pack(fill="x", padx=10, pady=5)
        
        self.btn_run = ttk.Button(ctrl_frame, text="RUN TEST", command=self.run_test, state="disabled")
        self.btn_run.pack(side="left", fill="x", expand=True, padx=5)
        
        self.btn_stop = ttk.Button(ctrl_frame, text="STOP", command=self.stop_test, state="disabled")
        self.btn_stop.pack(side="left", fill="x", expand=True, padx=5)
        
        self.btn_offload = ttk.Button(ctrl_frame, text="OFFLOAD OPTIMIZED MEMORY", command=self.offload_memory, state="disabled")
        self.btn_offload.pack(side="left", fill="x", expand=True, padx=5)

        self.btn_clear = ttk.Button(ctrl_frame, text="CLEAR MEMORY", command=self.clear_memory, state="disabled")
        self.btn_clear.pack(side="left", fill="x", expand=True, padx=5)
        
        # --- LOGGING AREA ---
        log_frame = ttk.LabelFrame(root, text="Device Log", padding=10)
        log_frame.pack(fill="both", expand=True, padx=10, pady=5)
        
        self.log_text = tk.Text(log_frame, height=10, state="disabled")
        self.log_text.pack(fill="both", expand=True)
        
        self.update_ui_state()

    def refresh_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports:
            self.port_combo.current(0)
            
    def log(self, msg):
        self.log_text.config(state="normal")
        self.log_text.insert("end", msg + "\n")
        self.log_text.see("end")
        self.log_text.config(state="disabled")

    def toggle_connection(self):
        if not self.is_connected:
            try:
                port = self.port_var.get()
                self.serial_port = serial.Serial(port, 115200, timeout=1)
                self.is_connected = True
                self.btn_connect.config(text="Disconnect")
                self.log(f"Connected to {port}")
                
                # Start listener thread
                self.listen_thread = threading.Thread(target=self.listen_serial, daemon=True)
                self.listen_thread.start()
                
                # Enable buttons
                self.btn_run.config(state="normal")
                self.btn_stop.config(state="normal")
                self.btn_offload.config(state="normal")
                self.btn_offload.config(state="normal")
                self.btn_clear.config(state="normal")
                self.btn_save_settings.config(state="normal")
                
                # Send Ping
                self.send_cmd("PING")
                
            except Exception as e:
                messagebox.showerror("Connection Error", str(e))
        else:
            if self.serial_port:
                self.serial_port.close()
            self.is_connected = False
            self.btn_connect.config(text="Connect")
            self.log("Disconnected")
            self.btn_run.config(state="disabled")
            self.btn_stop.config(state="disabled")
            self.btn_offload.config(state="disabled")
            self.btn_offload.config(state="disabled")
            self.btn_clear.config(state="disabled")
            self.btn_save_settings.config(state="disabled")

    def send_cmd(self, cmd):
        if self.is_connected and self.serial_port:
            full_cmd = cmd + "\n"
            self.serial_port.write(full_cmd.encode('utf-8'))
            self.log(f"> {cmd}")

    def listen_serial(self):
        while self.is_connected:
            try:
                if self.serial_port and self.serial_port.in_waiting:
                    line = self.serial_port.readline().decode('utf-8').strip()
                    if line:
                        # Use root.after to safely update UI from thread
                        self.root.after(0, self.log, f"< {line}")
            except Exception as e:
                print(f"Serial Error: {e}")
                break
            time.sleep(0.01)

    def update_ui_state(self):
        if self.test_type_var.get() == 2:
            self.entry_length.config(state="normal")
        else:
            self.entry_length.config(state="disabled")

    def run_test(self):
        # Configure first (ensure settings are up to date)
        if not self.upload_settings():
            return

        # Auto-Save before starting (Duplicate of manual save, but good for safety)
        self.send_cmd("SAVE_CONFIG")
        time.sleep(0.1)
        self.send_cmd("START")

    def save_settings(self):
        if self.upload_settings():
            self.send_cmd("SAVE_CONFIG")
            messagebox.showinfo("Success", "Settings Saved to Device Flash!")

    def upload_settings(self):
        t_type = self.test_type_var.get()
        self.send_cmd(f"SET_TYPE {t_type}")
        time.sleep(0.1)
        
        if t_type == 2:
            try:
                mins = float(self.length_var.get())
                self.send_cmd(f"SET_TIME {mins}")
                time.sleep(0.1)
            except ValueError:
                messagebox.showerror("Error", "Invalid Time Value")
                return False
        return True

    def stop_test(self):
        self.send_cmd("STOP")

    def clear_memory(self):
        if messagebox.askyesno("Confirm", "Are you sure you want to CLEAR the device memory? This cannot be undone."):
            self.send_cmd("CLEAR_FLASH")

    def offload_memory(self):
        # This function listens for a specific data stream
        # Disable listening thread temporarily or handle data parsing there?
        # For simplicity, we'll send the command and let the user see the log, 
        # OR we can prompt to save to file and then parse incoming data.
        
        file_path = filedialog.asksaveasfilename(defaultextension=".csv", filetypes=[("CSV Files", "*.csv")])
        if not file_path:
            return

        self.custom_receive_file = file_path
        self.send_cmd("READ_FLASH")
        # In a real app, we'd need a more robust state machine to distinct command responses from data dump.
        # Here, I'll trust the 'listen_serial' + 'log' is enough for debug, 
        # BUT the user wants a downloaded file.
        
        # WARNING: simple 'readline' in listen_serial sends everything to log.
        # Ideally, we should intercept "BEGIN_DATA" and start writing to file until "END_DATA".
        
        # Let's modify listen_serial to handle this mode.
        self.data_capture_mode = True
        self.captured_data = []

    # Modified listener to capture data
    def listen_serial(self):
        self.data_capture_mode = False
        self.captured_data = []
        
        while self.is_connected:
            try:
                if self.serial_port and self.serial_port.in_waiting:
                    line = self.serial_port.readline().decode('utf-8').strip()
                    if line:
                        if line == "BEGIN_DATA":
                            self.data_capture_mode = True
                            self.captured_data = []
                            self.root.after(0, self.log, "< Receiving Data...")
                        elif line == "END_DATA":
                            self.data_capture_mode = False
                            self.save_captured_data()
                            self.root.after(0, self.log, "< Data Transfer Complete")
                        elif self.data_capture_mode:
                            self.captured_data.append(line)
                        else:
                            self.root.after(0, self.log, f"< {line}")
            except Exception as e:
                print(f"Serial Error: {e}")
                self.is_connected = False
                break
            time.sleep(0.01)

    def save_captured_data(self):
        if hasattr(self, 'custom_receive_file') and self.custom_receive_file:
            try:
                with open(self.custom_receive_file, 'w', newline='') as f:
                    writer = csv.writer(f)
                    writer.writerow(["Index", "Voltage", "Current"]) # Header
                    for row in self.captured_data:
                        parts = row.split(',')
                        if len(parts) >= 1:
                            writer.writerow(parts)
                self.root.after(0, messagebox.showinfo, "Success", f"Data saved to {self.custom_receive_file}")
            except Exception as e:
                self.root.after(0, messagebox.showerror, "Error", f"Failed to save file: {e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = BioFETGUI(root)
    root.mainloop()

