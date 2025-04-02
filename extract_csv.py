import serial
import csv
import time
import os

# Define constants
SERIAL_PORT = "COM10"  # Change as per your system
BAUD_RATE = 9600
CSV_FOLDER = "data"
CSV_FILE = os.path.join(CSV_FOLDER, "pollution_data.csv")  # Ensure cross-platform compatibility

def ensure_folder_and_file():
    """Ensures the data folder and CSV file exist."""
    if not os.path.exists(CSV_FOLDER):
        os.makedirs(CSV_FOLDER)
        print(f"✅ Created folder: {CSV_FOLDER}")

    # If the CSV file doesn't exist, create it with headers
    if not os.path.isfile(CSV_FILE):
        with open(CSV_FILE, mode="w", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(["Time (s)", "Raw Sensor Value", "Voltage (V)", "Estimated CO2 (ppm)"])
        print(f"✅ Created file: {CSV_FILE}")

def read_serial_data():
    """Reads data from Arduino and writes to CSV file."""
    try:
        ensure_folder_and_file()  # Ensure file/folder exists before writing

        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Allow connection time
        print(f"✅ Connected to {SERIAL_PORT}")
        print(f"📂 Writing data to: {CSV_FILE}")

        with open(CSV_FILE, mode="a", newline="") as file:  # Append mode
            writer = csv.writer(file)

            while True:
                data = ser.readline().decode("utf-8").strip()  # Read data
                if data:
                    print(data)  # Print to console
                    values = parse_data(data)
                    if values:
                        writer.writerow(values)  # Write to CSV
                        file.flush()  # Ensure data is written immediately

    except serial.SerialException as e:
        print(f"❌ Serial Error: {e}")
    except KeyboardInterrupt:
        print("\n🛑 Stopping data collection.")
    finally:
        try:
            ser.close()  # Close serial port safely
        except NameError:
            pass  # If 'ser' was never initialized, avoid errors

def parse_data(data):
    """Parses a line of serial data and extracts values."""
    try:
        parts = data.split("|")
        time_sec = int(parts[0].split(":")[1].strip())
        raw_value = int(parts[1].split(":")[1].strip())
        voltage = float(parts[2].split(":")[1].strip().replace("V", ""))
        ppm = float(parts[3].split(":")[1].strip().replace("ppm", ""))

        return [time_sec, raw_value, voltage, ppm]
    except (IndexError, ValueError):
        return None  # Skip invalid lines

if __name__ == "__main__":
    read_serial_data()
