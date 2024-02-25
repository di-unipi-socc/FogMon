from model import get_updates, get_spec, get_reports, mongo, remove_reports_older_than
from .testbed import unify_reports
from .spec import dns_check4
import logging
from statistics import mean

def check_monitor(session):
    spec = get_spec(session)
    if spec["change_dates"] == []:
        if "monitor" in spec["specs"][-1]:
            if spec["specs"][-1]["monitor"] == True:
                if "nodes" in spec["specs"][-1]:
                    return spec["specs"][-1]["nodes"]
    return None


def compat(data, ips):
    logging.info([k for k in data])
    
    reports = data["Reports"]
    
    Leaders = data["Leaders"]["update"]["selected"]
    Ldr = {}
    Ids = {}
    Ips = {}

    logging.info([k for k in reports])
    for ldr, report in reports.items():
        for node in report["reports"]:
            def test_ip(ip, id):
                ip = dns_check4(ip,ips)
                if id not in Ids:
                    Ids[id] = "::1"
                if (ip is not None) and (ip not in Ips):
                    if ip not in ips:
                        logging.info(f"error extra ip {ip}")
                        return {"error": "extra ip"}
                    Ids[id] = ip
                    Ips[ip] = id
            src_id = node["source"]["id"]
            test_ip(node["source"]["ip"], src_id)
            Ldr[src_id] = node["leader"]
            
            def test_fun(test, T):
                dst_id = test["target"]["id"]
                test_ip(test["target"]["ip"], dst_id)
                
            for test in node["latency"]:
                test_fun(test,"L")
            for test in node["bandwidth"]:
                test_fun(test,"B")
    
    diff = [ip for ip in list(set(ips) - set(Ips.keys()))]
    logging.info(Ids)
    if len(diff) == 1:
        for id in Ids:
            if Ids[id] == "::1":
                Ids[id] = diff[0]
                Ips[diff[0]] = id 
    
    logging.info(diff)
    logging.info(Ids)
    logging.info(Leaders)
    logging.info(len(ips))
    logging.info(len(Ips))
    logging.info(len(Ids))

    hardware = {ip:{} for ip in Ips}
    links = {"L":{ip:{ip2:{"lasttime":-1} for ip2 in Ips if ip != ip2} for ip in Ips},"B":{ip:{ip2:{"lasttime":-1} for ip2 in Ips if ip != ip2} for ip in Ips}}

    for ldr, report in reports.items():
        for node in report["reports"]:
            src_id = node["source"]["id"]
            src_ip = Ids[node["source"]["id"]]
            src_ldr = Ldr[src_id]
            
            def test_fun(test, T):
                dst_id = test["target"]["id"]
                dst_ip = Ids[test["target"]["id"]]
                dst_ldr = Ldr[dst_id]

                if links[T][src_ip][dst_ip]["lasttime"] < test["lasttime"]:
                    val = {}
                    val["mean"] = test["mean"]
                    val["variance"] = test["variance"]
                    val["lasttime"] = test["lasttime"]
                    links[T][src_ip][dst_ip] = val
            
            if hardware[src_ip] == {}:
                hardware[src_ip] = node["hardware"]
            else:
                if hardware[src_ip]["lasttime"] < node["hardware"]["lasttime"]:
                    hardware[src_ip] = node["hardware"]


            for test in node["latency"]:
                test_fun(test,"L")
            for test in node["bandwidth"]:
                test_fun(test,"B")            
    
    return {"matrix":links, "hardware": hardware, "ips": ips}


def monitor(session):
    ips = check_monitor(session)
    if ips is None:
        return {"error": "no monitor: session not monitorable)"}
    
    logging.info("monitor True")

    updates = mongo.db.update.find({"session":session}, projection={'_id': False}).sort([("datetime", -1)])
    reports = mongo.db.reports.find({"session":session}, projection={'_id': False}).sort([("datetime", -1)])
    
    updates = list(updates)
    reports = list(reports)

    if len(updates) == 0:
        return {"error": "such empty: no update (nodes < 4 or too fast?)"}

    update = updates[0]
    selected = update["update"]["selected"]
    lasts = {}
    for leader in selected:
        for report in reports:
            if report["sender"]["id"] == leader["id"]:
                lasts[leader["id"]] = report["report"]
                break
        if leader["id"] not in lasts:
            return {"error": "such empty2: not found report about leader (too fast?)"}

    remove_reports_older_than(session, 600, reports[0]["datetime"])

    data = {"Reports":lasts,"Leaders":updates[0]}

    data = compat(data, ips)
    return data