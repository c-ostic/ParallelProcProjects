import os
import subprocess
import psutil
import time
import sys

def run_executable(executable_path, args):
    process = subprocess.Popen([executable_path] + args)
    process.wait()

def log_system_info():
    print("System Information:")
    print(f"CPU Cores: {psutil.cpu_count(logical=False)}")
    print(f"Logical CPUs: {psutil.cpu_count(logical=True)}")
    print(f"Total Memory: {psutil.virtual_memory().total / (1024 ** 3):.2f} GB")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python script_name.py input_file pattern_file thread_count")
        sys.exit(1)

    input_file = sys.argv[1]
    pattern_file = sys.argv[2]
    thread_count = sys.argv[3]

    mpiexec_path = "C:\\Program Files\\Microsoft MPI\\Bin\\mpiexec.exe"
    executable_name = ".\\x64\\Release\\Project3.exe"
    command_args = [input_file, pattern_file]

    current_directory = os.path.dirname(os.path.abspath(__file__))
    executable_path = os.path.join(current_directory, executable_name)

    # log_system_info()

    # Start the process
    process = subprocess.Popen([mpiexec_path, "-n", thread_count, executable_path] + command_args)
    pid = process.pid

    try:
        # Continuously monitor memory usage while the process is running
        while process.poll() is None:
            memory_usage = psutil.Process(pid).memory_info().rss / (1024 ** 2)  # in MB
            print(f"Memory Usage: {memory_usage:.2f} MB")
            time.sleep(0.0125)
    except psutil.NoSuchProcess:
        print("Process terminated.")

    print("Execution completed.")
