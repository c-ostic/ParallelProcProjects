import os
import subprocess
import psutil
import time

def run_executable(executable_path, args):
    process = subprocess.Popen([executable_path] + args)
    process.wait()

def log_system_info():
    print("System Information:")
    print(f"CPU Cores: {psutil.cpu_count(logical=False)}")
    print(f"Logical CPUs: {psutil.cpu_count(logical=True)}")
    print(f"Total Memory: {psutil.virtual_memory().total / (1024 ** 3):.2f} GB")

if __name__ == "__main__":
    executable_name = "Project2.exe"
    command_args = ["constitution.txt" , "24"]

    current_directory = os.path.dirname(os.path.abspath(__file__))
    executable_path = os.path.join(current_directory, executable_name)

    log_system_info()

    # Start the process
    process = subprocess.Popen([executable_path] + command_args)
    pid = process.pid

    try:
        # Continuously monitor memory usage while the process is running
        while True:
            memory_usage = psutil.Process(pid).memory_info().rss / (1024 ** 2)  # in MB
            print(f"Memory Usage: {memory_usage:.2f} MB")
            time.sleep(0.125)
    except psutil.NoSuchProcess:
        print("Process terminated.")

    print("Execution completed.")