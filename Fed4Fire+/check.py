#!/usr/bin/env python3
from clusterer import Clusterer
import json
import requests

if __name__ == "__main__":

    print("Download anew?")
    r = input()
    if r.lower() in ["y","yes"]:
        r = requests.get("http://131.114.72.76/data")
        data = r.json()["data"]
        with open("fogmon.json","w") as wr:
            json.dump(data,wr)
    else:
        with open("fogmon.json", 'r') as rd:
            data = json.load(rd)

    Leaders = data["Leaders"]["packet"]["data"]["selected"]
    Leaders = [l["ip"] for l in Leaders]
    L = len(Leaders)
    print(data["Reports"])
    print(Leaders)
    print(L)
    q_fogmon = data["Leaders"]["packet"]["data"]["cost"]
    with open("spec.json", 'r') as rd:
        spec = json.load(rd)
    
    Nodes = [k for k,v in spec["nodes"].items()]

    Links = {"L":{},"B":{}}
    for i in Nodes:
        Links["L"][i] = {}
        Links["B"][i] = {}
        for j in Nodes:
            Links["L"][i][j] = 0
            Links["B"][i][j] = 0

    for l,v in spec["links"].items():
        n1 = v["interfaces"][0].split(":")[0]
        n2 = v["interfaces"][1].split(":")[0]
        try:
            Links["L"][n1][n2] = v["latency"]
            Links["L"][n2][n1] = v["latency"]
            Links["B"][n1][n2] = v["capacity"]
            Links["B"][n2][n1] = v["capacity"]
        except:
            raise Exception("Some links are missing")

    # computing accuracy
    def get_node(node):
        return node["ip"]
    def accuracy_tests(tests, Links):
        rows = []
        for test in tests:
            n2 = get_node(test["target"])
            rows.append((test["mean"],test["variance"],Links[n2]))
        return rows
    
    rows_intra = {}
    rows_inter = {}
    for packet in data["Reports"]:
        rows_intra[get_node(packet["sender"])] = {"L":[],"B":[]}
        rows_inter[get_node(packet["sender"])] = {"L":[],"B":[]}
        print("Node:",packet["sender"])
        for report in packet["data"]["reports"]:
            if report["leader"] == packet["sender"]["id"]:
                rows = rows_intra[get_node(packet["sender"])]
            else:
                rows = rows_inter[get_node(packet["sender"])]
            print("Source:",report["source"],report["leader"])
            n1 = report["source"]["ip"]
            rows["L"] += accuracy_tests(report["latency"],Links["L"][n1])
            rows["B"] += accuracy_tests(report["bandwidht"],Links["B"][n1])
    import csv
    with open("links.csv", 'w') as csvfile:
        csvfile.writerow(["intralink"])
        csvfile.writerow(["latency"])
        for k,v in rows_intra:
            csvfile.writerow([k])
            csvfile.writerows(v["L"])
        csvfile.writerow(["bandwidth"])
        for k,v in rows_intra:
            csvfile.writerow([k])
            csvfile.writerows(v["B"])
        csvfile.writerow(["interlink"])
        csvfile.writerow(["latency"])
        for k,v in rows_inter:
            csvfile.writerow([k])
            csvfile.writerows(v["L"])
        csvfile.writerow(["bandwidth"])
        for k,v in rows_inter:
            csvfile.writerow([k])
            csvfile.writerows(v["B"])

    # computing cluster goodness
    probs = {}
    for i in Nodes:
        probs[i] = 0
    
    probs = {}
    qual = {}
    q_min = 100
    q_max = 0

    Num = 10000
    for _ in range(Num):
        clusterer = Clusterer([Nodes[0]],Nodes,Links["L"])
        
        data = clusterer.cluster()
        
        if len(data["new_leaders"]) != L:
            raise Exception("Wrong number of leaders!!!")
        
        #qual[tuple(data["new_leaders"])] = data["quality"]

        if q_min > data["quality"]:
            q_min = data["quality"]
        
        if q_max < data["quality"]:
            q_max = data["quality"]

        for l in data["new_leaders"]:
            probs[l] = probs.get(l,0) +1
    
    for i in probs:
        probs[i] /= Num
    

    q = (q_fogmon-q_min)/(q_max-q_min)
    print(q)
    print(q_max,q_fogmon,q_min)

    
    probs = {k: v for k, v in sorted(probs.items(), key=lambda item: item[1], reverse=True) if v != 0}
    print(probs)
    qual = {k: v for k, v in sorted(qual.items(), key=lambda item: item[1]) if v < 3}
    print(qual)