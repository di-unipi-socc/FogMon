import sys
import os
import socket

class Delay:
    def __init__(self):
        self.number = 0
        self.mapping = {}
        
    def delay(self, ip, ms):
        print("Delaying for", ms, "ms on", ip)
        self.mapping[ip] = ms
    
    def apply(self):
        # apply prio of n bands
        bands = max(len(self.mapping), 3)
        os.system(f"tc qdisc add dev eth0 root handle 1:0 prio bands {bands}")
        for i, ip in enumerate(self.mapping):
            i = i + 1
            # filter traffic to ip
            os.system(f"tc filter add dev eth0 parent 1:0 protocol ip prio 1 u32 match ip dst {ip} flowid 1:{i}")
            # delay traffic
            os.system(f"tc qdisc add dev eth0 parent 1:{i} handle {i+1}: netem delay {self.mapping[ip]}ms")

    def clear(self):
        self.mapping = {}
        os.system("tc qdisc del dev eth0 root")
    

if __name__ == "__main__":
    d = Delay()
    if len(sys.argv) == 2:
        if sys.argv[1] == "clear":
            d.clear()
            exit(0)
    if len(sys.argv) % 2 == 0:
        print("Usage: python delay.py <ip> <ms> <ip> <ms> ...")
        print("Usage: python delay.py clear")
        exit(1)
    mapping = {}
    
    for i in range(1, len(sys.argv), 2):
        ip = sys.argv[i]
        # resolve ip if not resolved
        try:
            ip = socket.gethostbyname(ip)
        except:
            pass

        mapping[ip] = sys.argv[i+1]

    for ip, ms in mapping.items():
        d.delay(ip, ms)
        
    d.apply()
    for ip in mapping:
        os.system(f"ping -c 2 {ip}")
