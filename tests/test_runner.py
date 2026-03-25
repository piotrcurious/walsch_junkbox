import os
import subprocess
import sys
import shutil

# Try to import tkinter, but don't fail if it's missing (e.g. in CI)
try:
    import tkinter as tk
    from tkinter import messagebox
    import threading
    TK_AVAILABLE = True
except ImportError:
    TK_AVAILABLE = False

# Get the directory of the current script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
MOCK_DIR = os.path.join(SCRIPT_DIR, "mock_arduino")
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "output")

if TK_AVAILABLE:
    class DisplayApp:
        def __init__(self, root, title):
            self.root = root
            self.root.title(title)
            self.canvas = tk.Canvas(root, width=128 * 4, height=64 * 4, bg="black")
            self.canvas.pack()
            self.pixels = []
            for y in range(64):
                row = []
                for x in range(128):
                    p = self.canvas.create_rectangle(x * 4, y * 4, (x + 1) * 4, (y + 1) * 4, outline="", fill="black")
                    row.append(p)
                self.pixels.append(row)

            self.serial_text = tk.Text(root, height=10, width=80, bg="black", fg="white")
            self.serial_text.pack()

        def update_display(self, hex_data):
            try:
                data = bytes.fromhex(hex_data)
                for i in range(len(data)):
                    b = data[i]
                    for bit in range(8):
                        idx = i * 8 + bit
                        if idx < 128 * 64:
                            x = idx % 128
                            y = idx // 128
                            color = "white" if (b & (1 << bit)) else "black"
                            self.canvas.itemconfig(self.pixels[y][x], fill=color)
            except Exception:
                pass

        def add_serial(self, text):
            self.serial_text.insert(tk.END, text + "\n")
            self.serial_text.see(tk.END)

    def run_sketch_interactive(bin_path, title):
        root = tk.Tk()
        app = DisplayApp(root, title)

        def process_output():
            process = subprocess.Popen([bin_path], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            for line in process.stdout:
                line = line.strip()
                if line.startswith("DISPLAY_FRAME: "):
                    app.update_display(line.replace("DISPLAY_FRAME: ", ""))
                else:
                    app.add_serial(line)
            process.wait()

        thread = threading.Thread(target=process_output, daemon=True)
        thread.start()
        root.mainloop()

def test_sketch(ino_file, interactive=False):
    print(f"Testing {ino_file}...")
    cpp_file = f"temp_{ino_file}.cpp"
    bin_file = f"test_{ino_file}.bin"
    try:
        ino_path = os.path.join(REPO_ROOT, ino_file)
        with open(ino_path, 'r') as f:
            ino_content = f.read()
        with open(os.path.join(MOCK_DIR, 'main.cpp'), 'r') as f:
            main_content = f.read()
        with open(cpp_file, 'w') as f:
            f.write("#include \"Arduino.h\"\n#include \"Wire.h\"\n#include \"Adafruit_SSD1306.h\"\n")
            if "bitCount" in ino_content and "int bitCount(" not in ino_content:
                f.write("#define bitCount(n) __builtin_popcount(n)\n")
            f.write(ino_content + "\n" + main_content)

        cmd = ["g++", "-I", MOCK_DIR, "-I", os.path.join(MOCK_DIR, "Adafruit_GFX"),
               "-I", os.path.join(MOCK_DIR, "Adafruit_SSD1306"), "-I", os.path.join(MOCK_DIR, "Wire"),
               "-o", bin_file, cpp_file]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Compilation FAILED for {ino_file}\n{result.stderr}")
            return False

        if not os.path.exists(OUTPUT_DIR):
            os.makedirs(OUTPUT_DIR)

        if interactive:
            if not TK_AVAILABLE:
                print("Error: tkinter not available for interactive mode.")
                return False
            run_sketch_interactive(f"./{bin_file}", f"OLED Mock: {ino_file}")
            return True
        else:
            # Clean old output for this sketch if it exists
            last_frame_path = os.path.join(OUTPUT_DIR, "last_frame.pbm")
            if os.path.exists(last_frame_path):
                os.remove(last_frame_path)

            run_result = subprocess.run([f"./{bin_file}"], capture_output=True, text=True)
            if run_result.returncode != 0:
                print(f"Execution FAILED for {ino_file}\n{run_result.stderr}")
                return False

            # Save the last frame for evaluation
            if os.path.exists(last_frame_path):
                shutil.copy(last_frame_path, os.path.join(OUTPUT_DIR, f"{ino_file}_result.pbm"))

            print(f"Execution SUCCESS for {ino_file}")
            return True
    finally:
        if os.path.exists(cpp_file): os.remove(cpp_file)
        if os.path.exists(bin_file): os.remove(bin_file)

def main():
    repo_root = os.getcwd() if REPO_ROOT == "" else REPO_ROOT
    sketches = sorted([f for f in os.listdir(repo_root) if f.endswith('.ino')])

    if len(sys.argv) > 1 and sys.argv[1] == "--interactive":
        if not TK_AVAILABLE:
            print("Error: tkinter not available.")
            sys.exit(1)
        print("Interactive mode: Select a sketch to run.")
        for i, s in enumerate(sketches):
            print(f"{i}: {s}")
        choice = input("Choice (or 'q' to quit): ")
        if choice.isdigit() and int(choice) < len(sketches):
            test_sketch(sketches[int(choice)], interactive=True)
    else:
        results = {s: test_sketch(s) for s in sketches}
        print("\nSummary:")
        for s, success in results.items():
            print(f"{s}: {'PASS' if success else 'FAIL'}")
        sys.exit(0 if all(results.values()) else 1)

if __name__ == "__main__":
    main()
