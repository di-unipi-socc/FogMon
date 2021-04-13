from inspect import CO_NESTED
import traceback
from fabric import Connection, Config
from fabric import task, SerialGroup, ThreadingGroup, runners
from fabric.exceptions import GroupException
from time import sleep
from enum import Enum
import threading
import os

import logging
logging.basicConfig()
logging.getLogger().setLevel(logging.FATAL)

class TestBeds(str, Enum):
    WALL1 = "urn:publicid:IDN+wall1.ilabt.iminds.be+authority+cm"
    WALL2 = "urn:publicid:IDN+wall2.ilabt.iminds.be+authority+cm"
    CITY = "urn:publicid:IDN+lab.cityofthings.eu+authority+cm"

class Ubuntu(str, Enum):
    WALL1 = "urn:publicid:IDN+wall1.ilabt.iminds.be+image+emulab-ops:UBUNTU18-64-STD"
    WALL2 = "urn:publicid:IDN+wall1.ilabt.iminds.be+image+emulab-ops:UBUNTU18-64-STD"
    CITY = "urn:publicid:IDN+lab.cityofthings.eu+image+emulab-ops:UBUNTU18-64-CoT-armgcc"

enable_nat = ["wget -O - -nv https://www.wall2.ilabt.iminds.be/enable-nat.sh | sudo bash"]

user = "marcog"

fogmon_images = [
    "diunipisocc/liscio-fogmon:test",
    "diunipisocc/liscio-fogmon:test2",
    "diunipisocc/liscio-fogmon:valgrind"
]

docker_script = [
    'sudo service ntp stop',
    'sudo ntpdate pool.ntp.org',
    "sudo apt-get -y update",
    "sudo DEBIAN_FRONTEND=noninteractive apt-get -y install apt-transport-https ca-certificates curl gnupg-agent software-properties-common",
    "sudo apt-get remove docker docker-engine docker.io containerd runc",
    "sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -",
    'sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"',
    "sudo apt-get update",
    "sudo apt-get install -y docker-ce docker-ce-cli containerd.io",
    f"sudo usermod -aG docker {user}",
    "sudo apt-get install -y bmon",
]

monitor_script = [
    "echo '' > test.log",
    "echo '' > bmon.log",
    "bmon -r 1 -o format:fmt='$(element:name) $(attr:rxrate:bytes) $(attr:txrate:bytes)\\n' -p $(ip route | grep default | sed -e 's/^.*dev.//' -e 's/.proto.*//') > bmon.log &",
    "P1=$!",
    "sudo docker stats --format '{{.Container}}\\t{{.CPUPerc}}\\t{{.MemUsage}}' > test.log &",
    "P2=$!",
    "wait $P1 $P2",
    "echo 'Done'"
]

class Testbed:

    def __init__(self, spec, path = "build"):
        self.config = Config(
            overrides={
                'ssh_config_path': f"./{path}/ssh-config",
                'load_ssh_configs': True,
                "run": {
                    "quiet": True,
                }
            }
        )
        self.conns = {node:Connection(node, config = self.config) for node in spec["nodes"]}
        os.chdir(path)

    def cleanup(self):
        try:
            for node,conn in self.conns.items():
                print(f"\rclosing {node}", end="")
                conn.close()
            print()
        except:
            pass
        

    def exec_script_(self, name, lines, node, no_script=False):        
        for _ in range(20):
            conn = self.conns[node]
            try:
                if no_script:
                    for line in lines:
                        conn.run(line,hide=True)
                    return
                file = f"/tmp/script-{name}.sh"
                conn.run(f"> {file}", hide=True)
                conn.run(f"chmod +x {file}", hide=True)
                script = ""
                for line in lines:
                    script += f"{line}\n"

                script = script.replace("\\","\\\\")
                script = script.replace("'","\\'")
                
                conn.run(f'sudo echo $\'{script}\' > {file}', hide=True)
                conn.run(f"screen -d -m -S {name} bash -c 'sudo {file}'")
                return
            except:
                pass
            sleep(1)
        print(f"bad {node}")

    def exec_scripts(self, name, scripts: dict, no_script=False):
        print(f"exec {name}")
        threads = []
        for node in scripts:
            thread = threading.Thread(target=self.exec_script_, args=(name,scripts[node], node),kwargs={"no_script":no_script})
            threads.append((thread,node))
        for (thread,node) in threads:
            thread.start()
        for (thread,node) in threads:
            print(f"join {node}\r", end="")
            thread.join()

    def exec_script(self, name, lines, nodes, no_script=False):
        scripts = {node:lines for node in nodes}
        self.exec_scripts(name,scripts,no_script)

    def get_file(self, node, src_name, dst_name):
        for i in range(10):
            conn = self.conns[node]
            try:
                conn.get(src_name, dst_name)
                print(f"{src_name}\r", end="")
                return
            except:
                pass
            sleep(5)

    def get_files(self, nodes: list, src_name: str, dst_name: str):
        threads = []
        for node in nodes:
            x = threading.Thread(target=self.get_file, args=(node,src_name, f"{node}-{dst_name}"))
            threads.append(x)
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()

    def wait_script_(self, name, command, node, retry = 0, timeout = 1):
        for i in range(10):
            conn = self.conns[node]
            try:
                i = 0
                while True:
                    result = conn.run(command, hide=True)
                    if int(result.stdout) == 1:
                        break
                    sleep(timeout)
                    if i>retry and retry != 0 :
                        return False
                    i += 1
                return True
            except:
                pass
        print("waiting failed!!!!!!!!!!")
        return False

    def wait_script(self, name, nodes, retry = 0, timeout = 1, condition = None):
        print(f"wait {name}")
        threads = []
        command = condition if condition is not None else f"screen -S {name} -Q select . > /dev/null 2>&1 ; echo $?"
        sleep(1)
        print(command)
        for node in nodes:
            x = threading.Thread(target=self.wait_script_, args=(name,command, node,retry,timeout))
            threads.append((x,node))
        for (thread,node) in threads:
            thread.start()
        for (thread,node) in threads:
            print(f"join {node}\r", end="")
            thread.join()
        print()

    def getIpv6s(self, spec):
        # resolve ips
        nodes = [node for node in spec["nodes"]]
        for i in range(10):
            try:
                conns = ThreadingGroup(*nodes,
                    config = self.config)
                
                results = conns.run("hostname -I", hide=True)
                for r in results:
                    ipv6 = results[r].stdout.split(" ")[-2]
                    spec["nodes"][r.original_host]["ipv6"] = ipv6
                break
            except GroupException as e:
                nodes = []
                for c, r in e.result.items():
                    if not isinstance(r,runners.Result) :
                        nodes.append(c.original_host)
                print(nodes)
            sleep(10)

    def generate_init_network_scripts(self, spec):
        scripts = {}
        for node in spec["nodes"]:
            comms = []
            comms.append(f"sudo sed -i -E 's/([a-z0-9-]+) ([a-zA-Z0-9-]+) ([a-zA-Z0-9-]+)$/\\3 \\1 \\2/' /etc/hosts")
            comms.append(f"sudo sed -i '/127.0.0.1\t{node}/d' /etc/hosts")
            comms.append(f'sudo bash -c \'echo "127.0.0.1\t{node}" >> /etc/hosts\'')
            if spec["nodes"][node]["testbed"] != TestBeds.CITY.value:
                for comm in enable_nat:
                    comms.append(comm)
            for l,v in spec["links"].items():
                n1 = v["interfaces"][0].split(":")[0]
                n2 = v["interfaces"][1].split(":")[0]
                found = False
                if n2 == node:
                    othername = n1
                    otherip = v["ips"][0]
                    found = True
                if n1 == node:
                    othername = n2
                    otherip = v["ips"][1]
                    found = True
                if found:
                    comms.append(f"sudo sed -i '/{otherip}/d' /etc/hosts")
                    comms.append(f'sudo bash -c \'echo "{otherip}\t{othername}" >> /etc/hosts\'')
            scripts[node] = comms
        return scripts
    
    def generate_network_scripts(self, spec):
        self.getIpv6s(spec)
        scripts = {}
        for node in spec["nodes"]:
            comms = []
            if spec["nodes"][node]["testbed"] != TestBeds.CITY.value:
                for comm in enable_nat:
                    comms.append(comm)
            for l,v in spec["links"].items():
                n1 = v["interfaces"][0].split(":")[0]
                n2 = v["interfaces"][1].split(":")[0]
                found = False
                if n2 == node:
                    othername = n1
                    myip = v["ips"][1]
                    otherip = v["ips"][0]
                    found = True
                if n1 == node:
                    othername = n2
                    myip = v["ips"][0]
                    otherip = v["ips"][1]
                    found = True
                grename = f"gre{othername}"
                myipv6 = spec["nodes"][node]["ipv6"]
                otheripv6 = spec["nodes"][othername]["ipv6"]
                if found:
                    comms.append(f"sudo ip -6 link del dev {grename} ; echo $?")
                    comms.append(f"sudo ip -6 link add name {grename} type ip6gre local {myipv6} remote {otheripv6} ttl 64")
                    comms.append(f"sudo ip link set up dev {grename}")
                    comms.append(f"sudo ip addr add {myip} peer {otherip} dev {grename}")
                    comms.append(f"sudo ip link set dev {grename} mtu 1400")
                    if "capacity" in v or "latency" in v or "packet_loss" in v:
                        command = f"sudo tc qdisc add dev {grename} root netem "
                        if "latency" in v:
                            latency = v["latency"]
                            if not v["same_testbed"] and spec["nodes"][node]["testbed"] != TestBeds.CITY.value and spec["nodes"][othername]["testbed"] != TestBeds.CITY.value:
                                latency -=0
                            elif not v["same_testbed"]:
                                latency -=4
                            elif v["testbed"] == TestBeds.CITY.value:
                                latency -=3
                            latency = latency//2
                            if latency < 0:
                                latency = 0
                            command+= f"delay {latency}ms "
                        if "capacity" in v:
                            capacity = v["capacity"]
                            command+= f"rate {capacity}kbit "
                        if "packet_loss" in v:
                            packet_loss = v["packet_loss"]
                            command+= f"loss random {packet_loss}% "
                        comms.append(command)
            scripts[node] = comms
        return scripts

    def setup(self, spec):
        nodes_filter = [node for node in spec["nodes"]] #[f"node{n}" for n in range(10)]
        nodes = [node for node in spec["nodes"] if node in nodes_filter]
        # setup network
        scripts = self.generate_init_network_scripts(spec)
        scripts = {k:v for k,v in scripts.items() if k in nodes_filter}
        self.exec_scripts("network-init",scripts)
        self.wait_script("network-init", nodes)

        scripts = self.generate_network_scripts(spec)
        scripts = {k:v for k,v in scripts.items() if k in nodes_filter}
        self.exec_scripts("network",scripts)
        self.wait_script("network", nodes)

        # setup docker
        self.exec_script("docker", docker_script, nodes)
        self.wait_script("docker", nodes,timeout=20)

        # pull fogmon and clean
        self.clean(nodes)
        self.pull(spec)

    def pull(self, spec):
        nodes = [node for node in spec["nodes"]]
        self.exec_script("pull", [f"sudo docker pull {fogmon_images[0]}"], nodes)
        self.wait_script("pull", nodes,timeout=5)

    def clean(self, nodes):
        self.exec_script("clean", ["sudo docker kill $(sudo docker ps -q)","sudo pkill -9 -f emulab-networkd.sh ; echo $?","sudo truncate -s 0 /var/log/syslog","sudo rm /var/log/syslog.*", "sudo rm -y log.txt", "sudo killall screen ; echo $?"], nodes)
        self.wait_script("clean", nodes)
        self.stop_monitor(nodes)

    def start(self, followers, leader, params, image=fogmon_images[0], only_followers = False):
        scripts = {node:[f"sudo docker run -it --net=host {image} -C {leader} {params} | tee log.txt"] for node in followers}
        if not only_followers:
            scripts[leader] = [f"sudo docker run -it --net=host {image} --leader {params} | tee log.txt"]
        self.exec_scripts("fogmon", scripts)


    def stop(self, nodes):
        self.exec_script("stop", [f"screen -S fogmon -X stuff '0'`echo -ne '\015'` | sudo docker ps"], nodes, no_script=True)
        return self.wait_script("stop", nodes, condition="screen -S fogmon -Q select . > /dev/null 2>&1 ; echo $?")

    def kill(self, nodes):
        self.exec_script("kill", ["sudo docker kill $(docker ps -q) | sudo docker ps"], nodes)
        return self.wait_script("stop", nodes, condition="screen -S fogmon -Q select . > /dev/null 2>&1 ; echo $?")

    def set_links(self, spec):
        nodes = [node for node in spec["nodes"]]
        scripts = self.generate_network_scripts(spec)
        self.exec_scripts("network",scripts)
        self.wait_script("network", nodes)

    def start_monitor(self, nodes):
        scripts = {node:monitor_script for node in nodes}
        self.exec_scripts("monitor",scripts)
    
    def collect_monitor(self, nodes):
        self.get_files(nodes, "test.log", "cpu.txt")
        self.get_files(nodes, "bmon.log", "bmon.txt")

    def stop_monitor(self, nodes):
        self.exec_script("monitor-stop", ["screen -S monitor -X stuff '0'`echo -ne $'\cc'` | screen -list | ps ax | grep 'bmon'"], nodes, no_script=True)
        print("monitor-stop")
        a =  self.wait_script("monitor-stop", nodes, condition="screen -S monitor -Q select . > /dev/null 2>&1 ; echo $?")
        print("monitor-stopped")
        return a
        