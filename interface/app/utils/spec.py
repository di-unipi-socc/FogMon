from model import mongo, get_leaders, get_lastreports, get_updates, get_spec, get_reports
import logging

def get_associations(session, reports=None):
    spec = get_spec(session)
    Nodes = [k for k,v in spec["specs"][0]["nodes"].items()]
    Nodes = {i:"None" for i in Nodes}

    if reports is None:
        reports = get_reports(session,limit=len(Nodes)*3)
    elif type(reports) == dict:
        reports = [v for k,v in reports.items()]
    
    
    for report in reports:
        for node in report["report"]["reports"]:
            for test in node["latency"]:
                ip = test["target"]["ip"]
                if ip in Nodes:
                    Nodes[ip] = test["target"]["id"]
            for test in node["bandwidth"]:
                ip = test["target"]["ip"]
                if ip in Nodes:
                    Nodes[ip] = test["target"]["id"]

    Ids = {}
    for k,v in Nodes.items():
        Ids[v] = k
        #logging.info("node: "+str(k)+" "+str(v))
    return Nodes,Ids

def associate_spec(reports, spec, session):
    Nodes,Ids = get_associations(session, reports)
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

    return {"nodes": Nodes,"ids": Ids, "links": Links, "spec": spec, "session": session}