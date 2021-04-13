#!/usr/bin/env python3
from io import FileIO
import json
import requests
import os

r = requests.get(f"http://131.114.72.76:8080/testbed")
data = r.json()
sessions = [x["session"] for x in data["data"]]
print(sessions)

sessions_present = {}

for file in os.listdir('./sessions/'):
    if "sessions" in file:
        with open(file, "r") as rd:
            print(file,flush=True)
            version = "2.0" if "2.0" in file else "1.1" if "links" in file else "1.0"
            modifier = "bugged" if "old" in file else ""
            print(version, modifier)
            js = json.load(rd)
            for el in js:
                sessions_present[el["session"]]= el
                sessions_present[el["session"]]["version"] = version
                sessions_present[el["session"]]["modifier"] = modifier
            

stats = {"b":0,"u":0,"g":0, "1": {"base and nodes": 0, "":0}}

not_sessions = [35, 36, 37, 38, 58, 59, 64, 65, 92, 93, 95, 96, 97, 98, 99, 100, 102, 112, 111, 103, 110, 106, 108, 121, 122, 123, 120, 124, 125, 28, 40, 148, 126, 187, 188, 189, 193, 195, 197, 183, 194, 196, 158, 161, 162, 164, 165, 163, 178, 179, 180, 181]

for id in sessions:
    if id in not_sessions:
        continue
    if id in sessions_present:
        print(id)
        name = sessions_present[id]["name"]
        modifier = sessions_present[id]["modifier"]
        if modifier == "":
            if "th" in name or "bad" in name:
                modifier = "bugged or repeated, "
        desc = modifier + name + " " + sessions_present[id]["version"] + "\n"

        for moment in sessions_present[id]["moments"]:
            desc += f"moment: {moment['moment']}, {moment['name']} \n"
        
        if modifier:
            stats["b"] +=1
        else:
            stats["g"] +=1

        r = requests.put(f"http://131.114.72.76:8080/testbed/{id}/desc", json={"desc":desc})
    else:
        stats["u"] +=1
        desc = "Unknown experiment"
        r = requests.put(f"http://131.114.72.76:8080/testbed/{id}/desc", json={"desc":desc})

print(stats)