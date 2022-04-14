from .accuracy import stabilities
from .spec import associate_spec

def reports_to_matrix(reports):
    links = {}
    leaders = {}
    ips = {}
    for leader_id, report in reports.items():
        for node in report["report"]["reports"]:
            src_id = node["source"]["id"]
            ldr_id = node["leader"]
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
                
                links[leader_id][T][src_id][dst_id] = val


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
        data_stabilities.append({"reports": reports_to_matrix(reports_change), "spec": spec})
    return data_stabilities