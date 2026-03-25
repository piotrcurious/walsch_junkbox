import os
import subprocess
import sys

# Get the directory of the current script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
MOCK_DIR = os.path.join(SCRIPT_DIR, "mock_arduino")
REPO_ROOT = os.path.dirname(SCRIPT_DIR)

def test_sketch(ino_file):
    print(f"Testing {ino_file}...")

    # Create a temporary cpp file combining the ino content and mock main
    cpp_file = f"temp_{ino_file}.cpp"
    try:
        ino_path = os.path.join(REPO_ROOT, ino_file)
        with open(ino_path, 'r') as f:
            ino_content = f.read()

        with open(os.path.join(MOCK_DIR, 'main.cpp'), 'r') as f:
            main_content = f.read()

        with open(cpp_file, 'w') as f:
            f.write(f"#include \"Arduino.h\"\n")
            f.write(f"#include \"Wire.h\"\n")
            f.write(f"#include \"Adafruit_SSD1306.h\"\n")
            # bitCount might be needed if not defined in the ino
            if "bitCount" in ino_content and "int bitCount(" not in ino_content:
                f.write("#define bitCount(n) __builtin_popcount(n)\n")
            f.write(ino_content)
            f.write("\n")
            f.write(main_content)

        # Compile with g++
        bin_file = f"test_{ino_file}.bin"
        cmd = ["g++", "-I", MOCK_DIR,
               "-I", os.path.join(MOCK_DIR, "Adafruit_GFX"),
               "-I", os.path.join(MOCK_DIR, "Adafruit_SSD1306"),
               "-I", os.path.join(MOCK_DIR, "Wire"),
               "-o", bin_file, cpp_file]

        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode != 0:
            print(f"Compilation FAILED for {ino_file}")
            print(result.stderr)
            return False
        else:
            print(f"Compilation SUCCESS for {ino_file}")
            # Run the binary
            run_result = subprocess.run([f"./{bin_file}"], capture_output=True, text=True)
            if run_result.returncode != 0:
                print(f"Execution FAILED for {ino_file}")
                print(run_result.stderr)
                return False
            else:
                print(f"Execution SUCCESS for {ino_file}")
                # Analyze output for crashes (implicit if it got here)
                # and check if it produced any Serial output
                if len(run_result.stdout) > 0:
                    print("Serial Output Preview (last 5 lines):")
                    lines = run_result.stdout.strip().split('\n')
                    for line in lines[-5:]:
                        print(f"  > {line}")
                return True
    finally:
        # Cleanup
        if os.path.exists(cpp_file):
            os.remove(cpp_file)
        bin_file = f"test_{ino_file}.bin"
        if os.path.exists(bin_file):
            os.remove(bin_file)

def main():
    # Sketches are in the repo root
    sketches = sorted([f for f in os.listdir(REPO_ROOT) if f.endswith('.ino')])
    results = {}
    for sketch in sketches:
        results[sketch] = test_sketch(sketch)

    print("\nSummary:")
    success_count = 0
    for sketch, success in results.items():
        status = "PASS" if success else "FAIL"
        print(f"{sketch}: {status}")
        if success:
            success_count += 1

    if success_count == len(sketches):
        print("\nAll sketches passed!")
        sys.exit(0)
    else:
        print(f"\n{len(sketches) - success_count} sketches failed.")
        sys.exit(1)

if __name__ == "__main__":
    main()
