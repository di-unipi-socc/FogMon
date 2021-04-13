from model import get_spec, save_footprint, get_footprints
import logging
import re

def compute_footprint(session):
    spec = get_spec(session)
    items = get_footprints(session)
    footprints = []
    logging.info([item["moment"] for item in items])
    for item in items:
        # calc mean bandwidth for every node
        moment = item["moment"]
        Nodes = [k for k,v in spec["specs"][moment]["nodes"].items()]
        datas = item["data"]
        footprint = {}
        footprint["cpu"] = {"max": 0, "min": 9999999999, "mean": 0}
        footprint["mem"] = {"max": 0, "min": 9999999999, "mean": 0}
        footprint["rx"] = {"max": 0, "min": 9999999999, "mean": 0}
        footprint["tx"] = {"max": 0, "min": 9999999999, "mean": 0}

        for k,v in footprint.items():
            time = 0
            for node,data in datas.items():
                for val in data[k]:
                    val = float(val)
                    if val > v["max"]:
                        v["max"] = val
                    if val < v["min"]:
                        v["min"] = val
                    v["mean"] += val
                time += len(data["cpu"])
            if time == 0:
                continue
            v["mean"] /= time
        footprints.append(footprint)
    return footprints

def save_footprints(files,session):
    spec = get_spec(session)
    moment = len(spec["specs"])-1
    Nodes = [k for k,v in spec["specs"][-1]["nodes"].items()]

    datas = {}

    for node in Nodes:
        logging.info(node)
        data = {"cpu":[],"mem":[],"rx":[],"tx":[]}
        
        r = files[node+"-cpu.txt"]
        for line in r.readlines():
            try:
                line = line[7:]
                line = line.decode()
                r = re.findall(f"\\t([0-9\\.]+)[%M]",line)
                if r:
                    cpu = r[0]
                    mem = r[1]
                    data["cpu"].append(cpu)
                    data["mem"].append(mem)
            except:
                pass

        r = files[node+"-bmon.txt"]
        for line in r.readlines():
            try:
                line = line.decode()
                r = re.findall(" ([0-9\\.]{2,})",line)
                if r:
                    rx = r[0]
                    tx = r[1]
                    data["rx"].append(rx)
                    data["tx"].append(tx)
            except:
                pass
        logging.info(len(data["cpu"]))
        logging.info(len(data["tx"]))
        data["tx"] = data["tx"][1:]
        data["rx"] = data["rx"][1:]
        while len(data["cpu"]) > len(data["tx"]):
            data["cpu"] = data["cpu"][1:]
            data["mem"] = data["mem"][1:]
        while len(data["tx"]) > len(data["cpu"]):
            data["tx"] = data["tx"][1:]
            data["rx"] = data["rx"][1:]
        logging.info(len(data["cpu"]))
        logging.info(len(data["tx"]))
        datas[node] = data
    save_footprint(session, moment, datas)
