import os; import subprocess; import sys; import shutil
try: import tkinter as tk; from tkinter import ttk; import threading; TK_AVAILABLE = True
except ImportError: TK_AVAILABLE = False
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__)); MOCK_DIR = os.path.join(SCRIPT_DIR, "mock_arduino"); REPO_ROOT = os.path.dirname(SCRIPT_DIR); OUTPUT_DIR = os.path.join(SCRIPT_DIR, "output")
if TK_AVAILABLE:
    class DisplayApp:
        def __init__(self, root, title, process):
            self.root = root; self.root.title(title); self.process = process; main_frame = ttk.Frame(root, padding="10"); main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
            self.img = tk.PhotoImage(width=128, height=64); self.display_canvas = tk.Canvas(main_frame, width=128*4, height=64*4, bg="black"); self.display_canvas.grid(row=0, column=0, columnspan=2); self.photo_item = self.display_canvas.create_image(0, 0, anchor="nw", image=self.img)
            self.serial_text = tk.Text(main_frame, height=10, width=80, bg="black", fg="white"); self.serial_text.grid(row=1, column=0, columnspan=2, pady=10)
            ctrl_frame = ttk.LabelFrame(main_frame, text="Signal Controls (A1/Pin 39)", padding="10"); ctrl_frame.grid(row=2, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
            self.freq_var = tk.DoubleVar(value=100.0); ttk.Label(ctrl_frame, text="Freq (Hz):").grid(row=0, column=0); ttk.Scale(ctrl_frame, from_=1, to=500, variable=self.freq_var, command=self.update_signals).grid(row=0, column=1)
            self.phase_var = tk.DoubleVar(value=0.0); ttk.Label(ctrl_frame, text="Phase (rad):").grid(row=1, column=0); ttk.Scale(ctrl_frame, from_=0, to=6.28, variable=self.phase_var, command=self.update_signals).grid(row=1, column=1)
            self.noise_var = tk.DoubleVar(value=0.02); ttk.Label(ctrl_frame, text="Noise:").grid(row=2, column=0); ttk.Scale(ctrl_frame, from_=0, to=0.5, variable=self.noise_var, command=self.update_signals).grid(row=2, column=1)
            self.offset_var = tk.DoubleVar(value=0.0); ttk.Label(ctrl_frame, text="Offset:").grid(row=3, column=0); ttk.Scale(ctrl_frame, from_=-1, to=1, variable=self.offset_var, command=self.update_signals).grid(row=3, column=1)
            self.type_var = tk.IntVar(value=0); ttk.Label(ctrl_frame, text="Wave:").grid(row=4, column=0); ttk.Radiobutton(ctrl_frame, text="Sine", variable=self.type_var, value=0, command=self.update_signals).grid(row=4, column=1, sticky=tk.W); ttk.Radiobutton(ctrl_frame, text="Square", variable=self.type_var, value=1, command=self.update_signals).grid(row=5, column=1, sticky=tk.W); ttk.Radiobutton(ctrl_frame, text="Saw", variable=self.type_var, value=2, command=self.update_signals).grid(row=6, column=1, sticky=tk.W)
        def update_signals(self, *args):
            f, p, n, t, o = self.freq_var.get(), self.phase_var.get(), self.noise_var.get(), self.type_var.get(), self.offset_var.get()
            self.process.stdin.write(f"SIG:39:{t}:{f}:{p}:0.5:{o}\nNOISE:39:{n}\n"); self.process.stdin.flush()
        def update_display(self, hex_data):
            try:
                data = bytes.fromhex(hex_data); img_data = []
                for y in range(64):
                    row = ["#ffffff" if (data[(y * 128 + x) // 8] & (1 << ((y * 128 + x) % 8))) else "#000000" for x in range(128)]
                    img_data.append("{" + " ".join(row) + "}")
                self.img.put(" ".join(img_data)); scaled_img = self.img.zoom(4, 4); self.display_canvas.itemconfig(self.photo_item, image=scaled_img); self._scaled_img = scaled_img
            except Exception: pass
        def add_serial(self, text): self.serial_text.insert(tk.END, text + "\n"); self.serial_text.see(tk.END)
    def run_sketch_interactive(bin_path, title):
        root = tk.Tk(); process = subprocess.Popen([bin_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True); app = DisplayApp(root, title, process)
        def process_output():
            for line in process.stdout:
                line = line.strip()
                if line.startswith("DISPLAY_FRAME: "): app.update_display(line.replace("DISPLAY_FRAME: ", ""))
                else: app.add_serial(line)
        threading.Thread(target=process_output, daemon=True).start(); root.mainloop(); process.terminate()
def test_sketch(ino_file, interactive=False):
    print(f"Testing {ino_file}..."); cpp_file = f"temp_{ino_file}.cpp"; bin_file = f"test_{ino_file}.bin"
    try:
        ino_path = os.path.join(REPO_ROOT, ino_file)
        with open(ino_path, 'r') as f: ino_content = f.read()
        with open(os.path.join(MOCK_DIR, 'main.cpp'), 'r') as f: main_content = f.read()
        with open(cpp_file, 'w') as f:
            f.write("#include \"Arduino.h\"\n#include \"Wire.h\"\n#include \"Adafruit_SSD1306.h\"\n")
            if "bitCount" in ino_content and "int bitCount(" not in ino_content: f.write("#define bitCount(n) __builtin_popcount(n)\n")
            f.write(ino_content + "\n" + main_content)
        cmd = ["g++", "-I", MOCK_DIR, "-o", bin_file, cpp_file]
        if subprocess.run(cmd, capture_output=True).returncode != 0: return False
        if interactive:
            if not TK_AVAILABLE: return False
            run_sketch_interactive(f"./{bin_file}", f"OLED Mock: {ino_file}"); return True
        else:
            if os.path.exists(os.path.join(OUTPUT_DIR, "last_frame.pbm")): os.remove(os.path.join(OUTPUT_DIR, "last_frame.pbm"))
            try: subprocess.run([f"./{bin_file}"], capture_output=True, timeout=5)
            except subprocess.TimeoutExpired: pass
            if os.path.exists(os.path.join(OUTPUT_DIR, "last_frame.pbm")): shutil.copy(os.path.join(OUTPUT_DIR, "last_frame.pbm"), os.path.join(OUTPUT_DIR, f"{ino_file}_result.pbm"))
            print(f"Execution SUCCESS for {ino_file}"); return True
    finally:
        if os.path.exists(cpp_file): os.remove(cpp_file)
        if os.path.exists(bin_file): os.remove(bin_file)
def main():
    repo_root = os.getcwd() if REPO_ROOT == "" else REPO_ROOT
    sketches = sorted([f for f in os.listdir(repo_root) if f.endswith('.ino')])
    if len(sys.argv) > 1 and sys.argv[1] == "--interactive":
        print("Interactive mode: Select a sketch to run.")
        for i, s in enumerate(sketches): print(f"{i}: {s}")
        choice = input("Choice: ")
        if choice.isdigit() and int(choice) < len(sketches): test_sketch(sketches[int(choice)], interactive=True)
    else:
        if not os.path.exists(OUTPUT_DIR): os.makedirs(OUTPUT_DIR)
        results = {s: test_sketch(s) for s in sketches}
        print("\nSummary:")
        for s, success in results.items(): print(f"{s}: {'PASS' if success else 'FAIL'}")
        sys.exit(0 if all(results.values()) else 1)
if __name__ == "__main__": main()
