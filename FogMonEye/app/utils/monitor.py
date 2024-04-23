from model import get_updates, get_spec, get_reports, mongo, remove_reports_older_than
from .testbed import unify_reports
from .spec import dns_check4
import logging
from statistics import mean

def check_monitor(session):
    # return None if not monitored (not an experiment, only monitoring)
    spec = get_spec(session)
    if spec["change_dates"] == []:
        if "monitor" in spec["data"]:
            if spec["data"]["monitor"] == True:
                if "nodes" in spec["data"]:
                    return spec["data"]["nodes"]
    return None


def compat(data, ips):
    logging.info([k for k in data])
    
    reports = data["Reports"]
    
    Leaders = data["Leaders"]["update"]["selected"]
    Ldr = {}
    Ids = {}
    Ips = {}
    ids = set()

    # fill Ids and Ips
    # Ips is a dict with ips as keys and ids as values
    # Ids is a dict with ids as keys and ips as values
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
            ids.add(src_id)
            test_ip(node["source"]["ip"], src_id)
            Ldr[src_id] = node["leader"]
            
            def test_fun(test, T):
                dst_id = test["target"]["id"]
                ids.add(dst_id)
                test_ip(test["target"]["ip"], dst_id)
                
            for test in node["latency"]:
                test_fun(test,"L")
            for test in node["bandwidth"]:
                test_fun(test,"B")
    
    # check if we did not match an ip, if is ::1, we can match it
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

    hardware = {src_id:{} for src_id in ids}
    links = {"L":{src_id:{dst_id:{"lasttime":-1} for dst_id in ids if src_id != dst_id} for src_id in ids},"B":{src_id:{dst_id:{"lasttime":-1} for dst_id in ids if src_id != dst_id} for src_id in ids}}

    for ldr, report in reports.items():
        for node in report["reports"]:
            src_id = node["source"]["id"]
            # src_ip = Ids[node["source"]["id"]]
            src_ldr = Ldr[src_id]
            
            def test_fun(test, T):
                dst_id = test["target"]["id"]
                # dst_ip = Ids[test["target"]["id"]]
                dst_ldr = Ldr[dst_id]

                if links[T][src_id][dst_id]["lasttime"] < test["lasttime"]:
                    val = {}
                    val["mean"] = test["mean"]
                    val["variance"] = test["variance"]
                    val["lasttime"] = test["lasttime"]
                    links[T][src_id][dst_id] = val
            
            if hardware[src_id] == {}:
                hardware[src_id] = node["hardware"]
            else:
                if hardware[src_id]["lasttime"] < node["hardware"]["lasttime"]:
                    hardware[src_id] = node["hardware"]


            for test in node["latency"]:
                test_fun(test,"L")
            for test in node["bandwidth"]:
                test_fun(test,"B")            

    return {"matrix":links, "hardware": hardware, "ids": list(ids)}


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

    spec = get_spec(session)

    data["extra"] = spec["extra"]

    return data