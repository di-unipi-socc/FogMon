from .accuracy import stabilities
from .spec import associate_spec
import logging

def reports_to_matrix(reports):
    links = {}
    leaders = {}
    ips = {}
    for leader_id, report in reports.items():
        for node in report["report"]["reports"]:
            src_id = node["source"]["id"]
            ldr_id = node["leader"]
            if src_id == "827fbb15-e0f8-45b1-a5ae-035388205801" or src_id == "0974e5b3-e902-4479-bf60-0a1c8be1a7fa":
                leaders[src_id] = ldr_id
            if ldr_id == leader_id:
                leaders[src_id] = ldr_id

    def init_links(links):
        for T in ["B","L"]:
            links[T] = {}
            for node in leaders:
                links[T][node] = {}
    
    for leader_id, report in reports.items():
        links[leader_id] = {}
        init_links(links[leader_id])

    for leader_id, report in reports.items():
        for node in report["report"]["reports"]:
            src_id = node["source"]["id"]
            src_ip = node["source"]["ip"]
            if src_ip not in ["::1","127.0.0.1"]:
                ips[src_id] = src_ip

            def test_fun(test, T):
                dst_id = test["target"]["id"]
                dst_ip = test["target"]["ip"]
                if (dst_id in ips) and (dst_ip not in ["::1","127.0.0.1"]):
                    ips[dst_id] = dst_ip
                val = {}
                val["mean"] = test["mean"]
                val["variance"] = test["variance"]
                val["lasttime"] = test["lasttime"]
                try:
                    links[leader_id][T][src_id][dst_id] = val
                except:
                    logging.info(str(leaders))
                    links[leader_id][T][src_id]
                    links[leader_id][T][src_id][dst_id]
                    raise

            for test in node["latency"]:
                test_fun(test,"L")
            for test in node["bandwidth"]:
                test_fun(test,"B")
    return {"matrix":links, "ips": ips, "leaders": leaders}

def export_stabilities(session):
    els = stabilities(session)
    data_stabilities = []
    for ((begin,end,reports_change,changes), spec) in els:
        spec = associate_spec(reports_change, spec, session)
        time = 0
        if end is not None:
            time = (end-begin).total_seconds()
        data_stabilities.append({"reports": reports_to_matrix(reports_change), "spec": spec, "time": time})
    return data_stabilities