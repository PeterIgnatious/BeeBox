import serial
from datetime import datetime

PORT = "COM3"
BAUDRATE = 115200

now = datetime.now()

dow = now.weekday() + 1

data = (
    f"{now.year} "
    f"{now.month} "
    f"{now.day} "
    f"{dow} "
    f"{now.hour} "
    f"{now.minute} "
    f"{now.second}\n"
)

with serial.Serial(PORT, BAUDRATE, timeout=1) as ser:
    ser.write(data.encode())

print("Data/hora enviada:", data.strip())