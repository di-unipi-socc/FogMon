from fabric import Connection, Config
from fabric import task, SerialGroup, ThreadingGroup
from enum import Enum
import json
import time

class TestBeds(str, Enum):
   WALL1 = "urn:publicid:IDN+wall1.ilabt.iminds.be+authority+cm"
   WALL2 = "urn:publicid:IDN+wall2.ilabt.iminds.be+authority+cm"
   CITY = "urn:publicid:IDN+lab.cityofthings.eu+authority+cm"

class Ubuntu(str, Enum):
   WALL1 = "urn:publicid:IDN+wall1.ilabt.iminds.be+image+emulab-ops:UBUNTU18-64-STD"
   WALL2 = "urn:publicid:IDN+wall1.ilabt.iminds.be+image+emulab-ops:UBUNTU18-64-STD"
   CITY = "urn:publicid:IDN+lab.cityofthings.eu+image+emulab-ops:UBUNTU18-64-CoT-armgcc"

user = "marcog"

enable_nat = ["wget -O - -nv https://www.wall2.ilabt.iminds.be/enable-nat.sh | sudo bash"]

images = [
    "diunipisocc/liscio-fogmon:test",
    "diunipisocc/liscio-fogmon:test2",
    "diunipisocc/liscio-fogmon:valgrind"
]

githubFogmon = [
    "git clone https://github.com/di-unipi-socc/FogMon-LiSCIo",
    "(cd FogMon-LiSCIo && git submodule init)",
    "(cd FogMon-LiSCIo && git submodule update)",
    "(cd FogMon-LiSCIo && sudo ./build.sh)"
]

docker = [
    'sudo service ntp stop',
    'sudo ntpdate pool.ntp.org',
    "sudo apt-get -y update",
    "sudo DEBIAN_FRONTEND=noninteractive apt-get -y install apt-transport-https ca-certificates curl gnupg-agent software-properties-common",
    "sudo apt-get remove docker docker-engine docker.io containerd runc",
    "curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -",
    'sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"',
    "sudo apt-get update",
    "sudo apt-get install -y docker-ce docker-ce-cli containerd.io",
    f"sudo usermod -aG docker {user}",
    #"newgrp docker"
    "sudo apt-get install -y bmon",
]

vbox = [
    'sudo wget -q https://www.virtualbox.org/download/oracle_vbox_2016.asc -O- | sudo apt-key add -',
    'echo "deb [arch=amd64] https://download.virtualbox.org/virtualbox/debian focal contrib" | sudo tee /etc/apt/sources.list.d/virtualbox.list',
    'sudo apt update',
    'sudo apt-get install --yes virtualbox',
    f'sudo usermod -aG vboxusers {user}',
    'echo virtualbox-ext-pack virtualbox-ext-pack/license select true | sudo debconf-set-selections',
    'sudo apt install -y virtualbox-ext-pack',
    'VBoxManage setproperty vrdeextpack "Oracle VM VirtualBox Extension Pack"',
    f'sudo chown {user}:vboxusers /mnt -R',
    'sudo apt install -y virtualbox-guest-additions-iso',
    #'wget -O /mnt/ubuntu-18.04.5-live-server-amd64.iso https://releases.ubuntu.com/18.04.5/ubuntu-18.04.5-live-server-amd64.iso'
    
    #'wget -O /mnt/Ubuntu_18.04.7z https://sourceforge.net/projects/osboxes/files/v/vb/59-U-u-svr/18.04/18.04.3/S18.04.3VB-64bit.7z/download',
    #'sudo apt install -y p7zip-full',
    #'7z e /mnt/Ubuntu_18.04.7z -o/mnt',
    #'rm /mnt/64bit -r',
    #'mv "/mnt/Ubuntu Server 18.04.3 (64bit).vdi" /mnt/Ubuntu_18.04.vdi',
]

startvbox = [
    'vboxmanage createvm --ostype Ubuntu_64 --basefolder "/mnt/virtualbox" --register --name "%s"',
    #'cp /mnt/Ubuntu_18.04.vdi /mnt/virtualbox/%s/Ubuntu_18.04.vdi',
    'vboxmanage modifyvm "%s" --memory 1024 --nic2 nat --vrde on --vrdeport 33890',
    'vboxmanage modifyvm "%s" --nic1 bridged --bridgeadapter1 enp1s0f0',
    'VBoxManage modifyvm  "%s" --natpf1 "guestssh,tcp,,2222,,22"',
    'vboxmanage createhd --filename "/mnt/virtualbox/%s/%s.vmdk" --format VMDK --size 16384 --variant stream',
    'vboxmanage storagectl "%s" --name "SATA" --add sata',
    'vboxmanage storageattach "%s" --storagectl SATA --port 0 --type hdd --medium "/mnt/virtualbox/%s/%s.vdi"',
    #'vboxmanage storageattach "%s" --storagectl SATA --port 0 --type hdd --medium "/mnt/virtualbox/%s/Ubuntu_18.04.vdi"',
    #'vboxmanage storageattach "%s" --storagectl SATA --port 15 --type dvddrive --medium /usr/share/virtualbox/VBoxGuestAdditions.iso',
    #'vboxmanage storageattach "%s" --storagectl SATA --port 15 --type dvddrive --medium /mnt/ubuntu-18.04.5-live-server-amd64.iso',
    'vboxmanage startvm "%s" --type headless',
    'VBoxManage controlvm "%s" poweroff',

]
# <emulab:blockstore name="bs1" size="60GB" mountpoint="/mnt" class="local"/>

# <hardware_type name="pcgen06"/>

# rdesktop localhost:33890

# sudo dhclient -r
# sudo dhclient
# sudo apt install openssh-server
# sudo apt install -y build-essential gcc make perl dkms
# sudo mount /dev/cdrom /mnt
# sudo /mnt/VBoxLinuxAdditions.run
# reboot

# VBoxManage guestproperty get <vmname> "/VirtualBox/GuestInfo/Net/0/V4/IP"
# vboxmanage clonehd file.vmdk clone.vmdk

# everyboot
# wget -O - -nv https://www.wall2.ilabt.iminds.be/enable-nat.sh | sudo bash



def staging(ctx):
    if "SPEC" not in ctx:
        with open("spec.json", 'r') as rd:
            ctx.SPEC = json.load(rd)
    spec = ctx.SPEC
    if "CONNS" not in ctx:
        config = Config(
            overrides={
                'ssh_config_path': "./ssh-config",
                'load_ssh_configs': True,
            }
        )
        nodes = [n for (n,v) in spec["nodes"].items()]
        conns = SerialGroup(*nodes,
            config = config)
        ctx.CONNS = conns
    if "TCONNS" not in ctx:
        config = Config(
            overrides={
                'ssh_config_path': "./ssh-config",
                'load_ssh_configs': True,
            }
        )
        nodes = [n for (n,v) in spec["nodes"].items()]
        conns = ThreadingGroup(*nodes,
            config = config)
        ctx.TCONNS = conns

def getIpv6s(ctx):
    # resolve ips
    spec = ctx.SPEC
    for conn in ctx.CONNS:
        out = conn.run("hostname -I")
        try:
            ipv6 = out.stdout.split(" ")[-2]
            spec["nodes"][conn.original_host]["ipv6"] = ipv6
        except:
            exit(1)

@task
def pingtest(ctx):
    staging(ctx)
    for conn in ctx.CONNS:
        conn.run('ping -c 3 8.8.8.8')

@task
def setupNetwork(ctx):
    print("setup network")
    staging(ctx)
    getIpv6s(ctx)
    spec = ctx.SPEC
    for conn in ctx.CONNS:
        comms = []
        print(spec["nodes"][conn.original_host])
        comms.append(f"sed -i -E 's/([a-z0-9-]+) ([a-zA-Z0-9-]+) ([a-zA-Z0-9-]+)$/\\3 \\1 \\2/' /etc/hosts")
        comms.append(f"sed -i '/127.0.0.1\t{conn.original_host}/d' /etc/hosts")
        comms.append(f'bash -c \'echo "127.0.0.1\t{conn.original_host}" >> /etc/hosts\'')
        if spec["nodes"][conn.original_host]["testbed"] != TestBeds.CITY.value:
            for comm in enable_nat:
                comms.append(comm)
        # if spec["nodes"][conn.original_host]["testbed"] == TestBeds.CITY.value:
        #     comms.append("sudo sudo ip link set dev enp2s0 mtu 1400")
        for l,v in spec["links"].items():
            n1 = v["interfaces"][0].split(":")[0]
            n2 = v["interfaces"][1].split(":")[0]
            found = False
            if n2 == conn.original_host:
                othername = n1
                grename = f"gre{othername}"
                myipv6 = spec["nodes"][n2]["ipv6"]
                otheripv6 = spec["nodes"][othername]["ipv6"]
                myip = v["ips"][1]
                otherip = v["ips"][0]
                found = True
            if n1 == conn.original_host:
                othername = n2
                grename = f"gre{othername}"
                myipv6 = spec["nodes"][n1]["ipv6"]
                otheripv6 = spec["nodes"][othername]["ipv6"]
                myip = v["ips"][0]
                otherip = v["ips"][1]
                found = True
            # if not v["same_testbed"] or v["testbed"] == TestBeds.CITY.value:
            if found:
                comms.append(f"ip -6 link add name {grename} type ip6gre local {myipv6} remote {otheripv6} ttl 64")
                comms.append(f"ip link set up dev {grename}")
                comms.append(f"ip addr add {myip} peer {otherip} dev {grename}")
                comms.append(f"ip link set dev {grename} mtu 1400")
                comms.append(f"sed -i '/{otherip}/d' /etc/hosts")
                comms.append(f'bash -c \'echo "{otherip}\t{othername}" >> /etc/hosts\'')
                if "capacity" in v or "latency" in v or "packet_loss" in v:
                    command = f"tc qdisc add dev {grename} root netem "
                    if "latency" in v:
                        latency = v["latency"]
                        if not v["same_testbed"]:
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
                    print(command)
        
        file = "/tmp/script6fdghf37ffa.sh"
        conn.run(f"> {file}")
        print(f"created {file}")
        conn.run(f"chmod +x {file}")
        line = ""
        for comm in comms:
            line += f"sudo {comm}\n"
        conn.run(f'echo \"{line}\" >> {file}')
        conn.run(f"screen -d -m -S network bash -c '{file}'")
        
@task
def removeNetwork(ctx):
    print("remove network")
    staging(ctx)
    spec = ctx.SPEC
    for conn in ctx.CONNS:
        comms = []
        print(spec["nodes"][conn.original_host])
        comms.append(f"sed -i '/127.0.0.1\t{conn.original_host}/d' /etc/hosts")
        comms.append(f"sed -i -E 's/([a-z0-9-]+) ([a-zA-Z0-9-]+) ([a-zA-Z0-9-]+)$/\\2 \\3 \\1/' /etc/hosts")
        for l,v in spec["links"].items():
            n1 = v["interfaces"][0].split(":")[0]
            n2 = v["interfaces"][1].split(":")[0]
            found = False
            if n2 == conn.original_host:
                grename = f"gre{n1}"
                otherip = v["ips"][0]
                found = True
            if n1 == conn.original_host:
                grename = f"gre{n2}"
                otherip = v["ips"][1]
                found = True
            if found:
                try:
                    comms.append(f"ip -6 link del dev {grename}")
                    comms.append(f"sed -i '/{otherip}/d' /etc/hosts")
                except:
                    pass
        file = "/tmp/scripd4h6ghf37ffa.sh"
        conn.run(f"> {file}")
        print(f"created {file}")
        conn.run(f"chmod +x {file}")
        line = ""
        for comm in comms:
            line += f"sudo {comm}\n"
        conn.run(f'echo \"{line}\" > {file}')
        conn.run(f"screen -d -m -S network bash -c '{file}'")

@task
def setupDocker(ctx):
    print("setup docker")
    staging(ctx)
    spec = ctx.SPEC
    conn = ctx.TCONNS

    file = "/tmp/script621f37ffa.sh"
    conn.run(f"> {file}", hide=True)
    conn.run(f"chmod +x {file}", hide=True)
    line = ""
    for comm in docker:
        line += f"{comm}\n"
    conn.run(f'echo \'{line}\' > {file}', hide=True)
    conn.run(f"screen -d -m -S docker bash -c '{file}'", hide=True)

    # for conn in ctx.CONNS:
    #     file = "/tmp/script621f37ffa.sh"
    #     conn.run(f"> {file}")
    #     print(f"created {file}")
    #     conn.run(f"chmod +x {file}")
    #     line = ""
    #     for comm in docker:
    #         line += f"{comm}\n"
    #     conn.run(f'echo \'{line}\' > {file}')
    #     conn.run(f"screen -d -m -S docker bash -c '{file}'")

@task
def waitSetupDocker(ctx):
    staging(ctx)
    conn = ctx.TCONNS
    while True:
        results = conn.run("screen -S docker -Q select . > /dev/null 2>&1 ; echo $?", hide=True)
        if len([r for r in results if int(results[r].stdout) != 1]) == 0:
            break
        print(len([r.original_host for r in results if int(results[r].stdout) != 1]),"\r",end="")
        time.sleep(1)
    print()
    print("end setup docker")

@task
def setupAll(ctx):
    staging(ctx)
    setupNetwork(ctx)
    conn = ctx.TCONNS
    while True:
        results = conn.run("screen -S network -Q select . > /dev/null 2>&1 ; echo $?", hide=True)
        if len([r for r in results if int(results[r].stdout) != 1]) == 0:
            break
        time.sleep(1)
    print("end setup network")

    setupDocker(ctx)
    waitSetupDocker(ctx)
    
    pullFogmon(ctx)
    waitPullFogmon(ctx)

@task
def setupVbox(ctx):
    staging(ctx)
    spec = ctx.SPEC
    for conn in ctx.CONNS:
        for n,v in spec["nodes"].items():
            if n == conn.original_host:
                file = "/tmp/script6df5dfa.sh"
                conn.run(f"> {file}")
                print(f"created {file}")
                conn.run(f"chmod +x {file}")
                for comm in vbox:
                    conn.run(f'echo \'{comm}\' > {file}')
                conn.run(f"screen -d -m -S vbox bash -c '{file}'")

@task
def pullFogmon(ctx):
    print("pull fogmon")
    staging(ctx)
    conn = ctx.TCONNS

    i=0
    for image in images[1:2]:
        # sudo docker pull diunipisocc/liscio-fogmon:test
        conn.run(f"screen -d -m -S fogmon-{i} bash -c 'sudo docker pull {image}'", hide=True)
        i+=1

    # for conn in ctx.CONNS:
    #     i=0
    #     for image in images[:1]:
    #         conn.run(f"screen -d -m -S fogmon-{i} bash -c 'sudo docker pull {image}'")
    #         i+=1
    #     print(conn.original_host)

@task
def waitPullFogmon(ctx):
    staging(ctx)
    conn = ctx.TCONNS
    for i in range(len(images)):
        while True:
            results = conn.run(f"screen -S fogmon-{i} -Q select . > /dev/null 2>&1 ; echo $?", hide=True)
            if len([r for r in results if int(results[r].stdout) != 1]) == 0:
                break
            print(len([r.original_host for r in results if int(results[r].stdout) != 1]),"\r",end="")
            time.sleep(1)
    print()
    print("end")

@task
def buildFogmon(ctx):
    staging(ctx)

    conn = ctx.TCONNS
    file = "/tmp/script621f37ffa.sh"
    conn.run(f"> {file}", hide=True)
    conn.run(f"chmod +x {file}", hide=True)
    line = ""
    for comm in githubFogmon:
        line += f"{comm}\n"
    conn.run(f'echo \'{line}\' > {file}', hide=True)
    conn.run(f"screen -d -m -S githubFogmon bash -c '{file}'", hide=True)

    # for conn in ctx.CONNS:
    #     file = "/tmp/script621f3765a.sh"
    #     conn.run(f"> {file}")
    #     print(f"created {file}")
    #     conn.run(f"chmod +x {file}")
    #     line = ""
    #     for comm in githubFogmon:
    #         line += f"{comm}\n"
    #     conn.run(f'echo \'{line}\' > {file}')
    #     conn.run(f"screen -d -m -S githubFogmon bash -c '{file}'")
    #     print(conn.original_host)

@task
def startFogmon(ctx):
    staging(ctx)
    leader = None
    session = ""
    if "session" in ctx.SPEC:
        session = "-s "+str(ctx.SPEC["session"])+" -i 131.114.72.76:8080"
    for conn in ctx.CONNS:
        if leader is None:
            # sudo docker run -it --net=host diunipisocc/liscio-fogmon:test --leader
            conn.run(f"screen -d -m -S fogmon bash -c 'sudo docker run -it --net=host {images[1]} --leader {session} | tee log.txt'")
            leader = conn.original_host
        else:
            # sudo docker run -it --net=host diunipisocc/liscio-fogmon:test -C node0
            conn.run(f"screen -d -m -S fogmon bash -c 'sudo docker run -it --net=host {images[1]} -C {leader} {session} | tee log.txt'")
        print(conn.original_host)

@task
def startFogmonDefault(ctx):
    staging(ctx)
    leader = None
    param_dict = {
        "--time-report": 30,
        "--time-tests": 30,
        "--leader-check": 8,
        "--time-latency": 30,
        "--time-bandwidth": 600,
        "--heartbeat": 90,
        "--time-propagation": 20,
        "--max-per-latency": 100,
        "--max-per-bandwidth": 3,
        "--sensitivity": 15,
        "--hardware-window": 20,
        "--latency-window": 10,
        "--bandwidth-window": 5,
        "-t": 5,
    }
    if "session" in ctx.SPEC:
        param_dict["-s"] = ctx.SPEC["session"]
        param_dict["-i"] = "131.114.72.76:8080"
    params = ""
    for param,val in param_dict.items():
        params += f"{param} {val} "
    for conn in ctx.CONNS:
        if leader is None:
            # sudo docker run -it --net=host diunipisocc/liscio-fogmon:test --leader
            conn.run(f"screen -d -m -S fogmon bash -c 'sudo docker run -it --net=host --cap-add=NET_ADMIN {images[1]} --leader {params} | tee log.txt'")
            leader = conn.original_host
        else:
            # sudo docker run -it --net=host diunipisocc/liscio-fogmon:test -C node0
            conn.run(f"screen -d -m -S fogmon bash -c 'sudo docker run -it --net=host --cap-add=NET_ADMIN {images[1]} -C {leader} {params} | tee log.txt'")
        print(conn.original_host)

@task
def startFogmonValgrind(ctx):
    staging(ctx)
    leader = None
    session = ""
    if "session" in ctx.SPEC:
        session = "-s "+str(ctx.SPEC["session"])+" -i 131.114.72.76:8080"
    for conn in ctx.CONNS:
        if leader is None:
            # sudo docker run -it --net=host diunipisocc/liscio-fogmon:test --leader -s 3 -i 131.114.72.76:8080
            conn.run(f"screen -d -m -S fogmon bash -c 'sudo docker run -it --net=host --cap-add=NET_ADMIN {images[2]} --leader {session} | tee log.txt'")
            leader = conn.original_host
        else:
            # sudo docker run -it --net=host diunipisocc/liscio-fogmon:test -C node0
            conn.run(f"screen -d -m -S fogmon bash -c 'sudo docker run -it --net=host --cap-add=NET_ADMIN {images[2]} -C {leader} {session} | tee log.txt'")
        print(conn.original_host)

@task
def startMonitor(ctx):
    script = """
echo '' > test.log
echo '' > bmon.log
bmon -r 1 -o format:fmt='$(element:name) $(attr:rxrate:bytes) $(attr:txrate:bytes)\\n' -p $(ip route | grep default | sed -e 's/^.*dev.//' -e 's/.proto.*//') > bmon.log &
P1=$!
sudo docker stats --format '{{.Container}}\\t{{.CPUPerc}}\\t{{.MemUsage}}' > test.log &
P2=$!
wait $P1 $P2
echo 'Done'
    """
    """sudo docker stats --format '{{.Container}}\\t{{.CPUPerc}}\\t{{.MemUsage}}' > stats.txt
    psrecord $(pgrep dockerd) --interval 1 --log test.log
    """
    script = script.replace("$","\\$")
    staging(ctx)
    spec = ctx.SPEC
    for conn in ctx.CONNS:
        file = "/tmp/script6409f4fa.sh"
        conn.run(f"> {file}")
        print(f"created {file}")
        conn.run(f"chmod +x {file}")
        conn.run(f"echo \"{script}\" > {file}")
        conn.run(f"screen -d -m -S monitor bash -c '{file}'")
    pass


@task
def stopMonitor(ctx):
    staging(ctx)
    for conn in ctx.CONNS:
        conn.run("screen -S monitor -X stuff '0'`echo -ne $'\cc'` | screen -list | ps ax | grep 'bmon'")
    for conn in ctx.CONNS:
        host = conn.original_host
        conn.get("test.log", host+"-cpu.txt")
        conn.get("bmon.log", host+"-bmon.txt")
        print(conn.original_host)

@task
def sendFootprint(ctx):
    staging(ctx)
    spec = ctx.SPEC
    files = {}
    for conn in ctx.CONNS:
        host = conn.original_host
        bmon = host+"-bmon.txt"
        psrecord = host+"-cpu.txt"
        files[bmon] = open(bmon,'rb')
        files[psrecord] = open(psrecord,'rb')
        print(conn.original_host)

    import requests
    r = requests.post("http://131.114.72.76:8080/testbed/%d/footprint"%spec["session"], files=files)
    print(r.status_code)
    print(r.json)


@task
def clearFogmon(ctx):
    staging(ctx)
    for conn in ctx.CONNS:
        host = conn.original_host
        conn.run("rm log.txt")
        print(host)

@task
def gatherFogmon(ctx):
    staging(ctx)
    for conn in ctx.CONNS:
        host = conn.original_host
        conn.get("log.txt", host+"-log.txt")
        print(host)

@task
def stopFogmon(ctx):
    staging(ctx)

    conn = ctx.TCONNS
    conn.run("screen -S fogmon -X stuff '0'`echo -ne '\015'` | sudo docker ps")
    for i in range(30):
        results = conn.run("screen -S fogmon -Q select . > /dev/null 2>&1 ; echo $?", hide=True)
        if len([r for r in results if int(results[r].stdout) != 1]) == 0:
            return
        print("Retry...",end=" ",flush=True)
        time.sleep(1)
    print("Not terminated")

    # for conn in ctx.CONNS:
    #     conn.run("screen -S fogmon -X stuff '0'`echo -ne '\015'` | docker ps")


def genSession(spec):
    spec["id"] += 1
    import requests
    r = requests.post("http://131.114.72.76:8080/testbed", json=spec)
    if r.status_code != 201:
        raise Exception("connection error")
    session = r.json()["session"]
    spec["session"] = session
    print("session",session)        
    return session

def removeData(session):
    import requests
    r = requests.get(f"http://131.114.72.76:8080/testbed/{session}/remove")
    if r.status_code != 200:
        raise Exception("connection error")

def generateMoment(session, spec):
    import requests
    r = requests.post(f"http://131.114.72.76:8080/testbed/{session}", json=spec)
    if r.status_code != 201:
        raise Exception("connection error")

def startFogmonParams(config, ctx):
    leader = None
    if "session" in ctx.SPEC:
        config["-s"] = ctx.SPEC["session"]
        config["-i"] = "131.114.72.76:8080"
    params = ""
    for param,val in config.items():
        params += f"{param} {val} "
    for conn in ctx.CONNS:
        if leader is None:
            # sudo docker run -it --net=host diunipisocc/liscio-fogmon:test --leader
            conn.run(f"screen -d -m -S fogmon bash -c 'sudo docker run -it --net=host {images[1]} --leader {params} | tee log.txt'")
            leader = conn.original_host
        else:
            # sudo docker run -it --net=host diunipisocc/liscio-fogmon:test -C node0
            conn.run(f"screen -d -m -S fogmon bash -c 'sudo docker run -it --net=host {images[1]} -C {leader} {params} | tee log.txt'")
        print(conn.original_host)

def waitStability(session):
    import requests
    stable = False
    while not stable:
        time.sleep(60)
        try:
            r = requests.get(f"http://131.114.72.76:8080/testbed/{session}/accuracy")
            if r.status_code != 200:
                raise Exception("connection error")
            moments = r.json()["data"]
            moment = moments[-1]
            if "yes" in moment["stable"]:
                stable = True
        except:
            pass

def getLeaders(session):
    import requests
    r = requests.get(f"http://131.114.72.76:8080/testbed/{session}")
    if r.status_code != 200:
        raise Exception("connection error")
    data = r.json()["data"]
    leaders = [v["ip"] for v in data["Leaders"]["update"]["selected"]]
    return leaders
    

def killLeaders(num, ctx, moment=True):
    spec = ctx.SPEC
    session = spec["session"]
    
    leaders = getLeaders(session)
    import random
    toKill = random.sample(leaders,num)
    
    #remove leaders

    if moment:
        generateMoment(session, spec)

    config = Config(
        overrides={
            'ssh_config_path': "./ssh-config",
            'load_ssh_configs': True,
        }
    )
    conns = ThreadingGroup(*toKill,
            config = config)

    conns.run("screen -S fogmon -X stuff '0'`echo -ne '\015'` | sudo docker ps")
    for i in range(30):
        results = conns.run("screen -S fogmon -Q select . > /dev/null 2>&1 ; echo $?", hide=True)
        if len([r for r in results if int(results[r].stdout) != 1]) == 0:
            return
        time.sleep(1)

def killLeadersExcept(num, ctx, moment=True):
    pass

def killNodes(num, ctx, moment=True):
    pass

def restoreNodes(nodes, ctx, moment=True):
    pass

def restrictBandwidth(percentage, bandwidthMB, ctx, moment=True):
    pass

def increaseLatency(percentage, latency, ctx, moment=True):
    pass

def isolateGroups(num, ctx, moment=True):
    pass

def restoreLinks(links, ctx):
    pass

@task
def startExperiments(ctx):
    staging(ctx)

    num = len(ctx.SPEC["nodes"])
    print(f"nodes: {num}")
    
    configs = {
        "default": {   
            "--time-report": 30,
            "--time-tests": 30,
            "--leader-check": 8,
            "--time-latency": 30,
            "--time-bandwidth": 600,
            "--heartbeat": 90,
            "--time-propagation": 20,
            "--max-per-latency": 100,
            "--max-per-bandwidth": 3,
            "--sensitivity": 15,
            "--hardware-window": 20,
            "--latency-window": 10,
            "--bandwidth-window": 5,
            "-t": 5,
        },
        "reactive": {   
            "--time-report": 20,
            "--time-tests": 20,
            "--leader-check": 4,
            "--time-latency": 20,
            "--time-bandwidth": 300,
            "--heartbeat": 60,
            "--time-propagation": 10,
            "--max-per-latency": 100,
            "--max-per-bandwidth": 3,
            "--sensitivity": 10,
            "--hardware-window": 10,
            "--latency-window": 5,
            "--bandwidth-window": 3,
            "-t": 5,
        },
        "more reactive": {   
            "--time-report": 10,
            "--time-tests": 10,
            "--leader-check": 2,
            "--time-latency": 10,
            "--time-bandwidth": 60,
            "--heartbeat": 30,
            "--time-propagation": 5,
            "--max-per-latency": 100,
            "--max-per-bandwidth": 3,
            "--sensitivity": 5,
            "--hardware-window": 5,
            "--latency-window": 3,
            "--bandwidth-window": 3,
            "-t": 5,
        }
    }
    sessions = {}

    for name,config in configs.items():
        session = genSession(ctx.SPEC)
        sessions[session] = {"num": num, "name": name, "version": 1}
        removeData(session)

        startFogmonParams(config, ctx)
        waitStability(session)

        els = killLeaders(1, ctx)
        waitStability(session)
        restoreNodes(els, ctx)
        waitStability(session)

        els = killLeadersExcept(1, ctx)
        waitStability(session)
        restoreNodes(els, ctx)
        waitStability(session)

        els = killNodes(num//4, ctx)
        waitStability(session)
        restoreNodes(els, ctx)
        waitStability(session)

        els = killNodes(num//2, ctx)
        waitStability(session)
        restoreNodes(els, ctx)
        waitStability(session)
        stopFogmon(ctx)

        with open("build/sessions.json","w") as wr:
            json.dump(sessions, wr)
        
        links = restrictBandwidth(10,0.1, ctx, moment=False)
        session = genSession(ctx.SPEC)
        sessions[session] = {"num": num, "name": name, "version": 1}
        removeData(session)

        startFogmonParams(config)
        waitStability(session)
        restoreLinks(links, ctx)
        waitStability(session)

        links = increaseLatency(25,500, ctx)
        waitStability(session)
        restoreLinks(links, ctx)
        waitStability(session)

        links = isolateGroups(1, ctx)
        waitStability(session)
        restoreLinks(links, ctx)
        waitStability(session)
        stopFogmon(ctx)

        with open("build/sessions.json","w") as wr:
            json.dump(sessions, wr)
