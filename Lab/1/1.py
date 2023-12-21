import subprocess
import time
from scapy.all import *

host = "127.0.0.1"
port = 10495

nc_command = f"nc -u {host} {port}"
nc_process = subprocess.Popen(nc_command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
time.sleep(3)

nc_process.stdin.write("hello 110700045"+"\n")
nc_process.stdin.flush()
response = nc_process.stdout.readline()
response=response[3:]
cmd='chals '+response

tcp_process=subprocess.Popen('sudo tcpdump -i any udp port 10495 -w 2.pcap\n', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
time.sleep(3)

nc_process.stdin.write(cmd+"\n")
nc_process.stdin.flush()

while True:
    res = nc_process.stdout.readline()

    print(res, end="")

    if 'END FLAG' in res:
        break

time.sleep(3)

tcp_process.stdin.write('\x03\n')
tcp_process.stdin.flush()

ans=""
pcap_file = '2.pcap'
packets = rdpcap(pcap_file)
for packet in packets:
    total_length = len(packet)-48
    if 0<=total_length and total_length<=127:
        ans+=chr(total_length)

start_index = ans.find("FLAG{")
end_index = ans.find("}", start_index)
if start_index != -1 and end_index != -1:
    extracted_string = ans[start_index:end_index + 1]

answer='verfy '+extracted_string
nc_process.stdin.write(answer+'\n')
nc_process.stdin.flush()

while True:
    judge=nc_process.stdout.readline()
    
    if "GOOD JOB!" in judge:
        print(answer)
        print(judge, end="")
        break

    else:
        print(judge, end="")


nc_process.stdin.close()
nc_process.stdout.close()
nc_process.stderr.close()
tcp_process.stdin.close()
tcp_process.stdout.close()
tcp_process.stderr.close()
