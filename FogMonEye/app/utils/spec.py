from model import mongo, get_leaders, get_updates, get_spec, get_reports
import logging
import dns.resolver
import ipaddress

def dns_check4(ip, DSTs):
    if ip == "::1":
        return None
    try:
        ip = ipaddress.ip_address(ip)
        try:
            ip = str(ip.ipv4_mapped)
        except:
            ip = str(ip)
    except:
        if "." in ip:
            result = dns.resolver.query(ip, 'A')
            for val in result:
                logging.info(f"{val} {ip}")
                val = str(val)
                if val in DSTs:
                    return val
    if ip in DSTs:
        return ip
    logging.info("None "+ip)
    return None
        

def get_associations(session, reports=None):
    try:
        spec = get_spec(session)
        if type(spec["specs"][0]["nodes"]) is list:
            Nodes = [k for k in spec["specs"][0]["nodes"]]
        else:
            Nodes = [k for k,v in spec["specs"][0]["nodes"].items()]
        Nodes = {i:"None" for i in Nodes}
    except:
        Nodes = {}
        spec = None
    Nodes_ = {}
    if reports is None:
        reports = get_reports(session,limit=len(Nodes)*3+1)
    elif type(reports) == dict:
        reports = [v for k,v in reports.items()]
    
    
    for report in reports:
        for node in report["report"]["reports"]:
            for test in node["latency"]:
                ip = test["target"]["ip"]
                Nodes_[ip] = test["target"]["id"]
            for test in node["bandwidth"]:
                ip = test["target"]["ip"]
                Nodes_[ip] = test["target"]["id"]
    if spec is not None:
        for ip,id in Nodes_.items():
            ip = dns_check4(ip,Nodes)
            if ip is not None:
                Nodes[ip] = id
    else:
        Nodes = Nodes_

    Ids = {}
    for k,v in Nodes.items():
        Ids[v] = k
        logging.info("node: "+str(k)+" "+str(v))
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