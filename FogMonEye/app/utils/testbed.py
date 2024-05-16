from .spec import get_associations
from model import mongo
from datetime import datetime
from bson.son import SON
import json
from collections import OrderedDict
from model import clean_results, deaggregate, get_spec, remove_reports_older_than, remove_updates_older_than
import logging

def get_sessions():
    specs = mongo.db.spec.find({}, projection={'_id': False})
    sessions = mongo.db.reports.aggregate([
        {"$sort": SON([("datetime", -1)])},
        {"$group": {
            "_id": {"session": "$session"},
            #"sender": "$sender",
            "datetime": {"$first": "$datetime"},
            #"report": {"$first": "$report"}
        }}
    ])
    specs = clean_results(specs)
    sessions = deaggregate(sessions)

    sessions = [v for v in sorted(sessions, key=lambda item: item["datetime"])]

    # concat specs and sessions
    for spec in specs:
        for session in sessions:
            if session["session"] == spec["session"]:
                break
        else:
            sessions.append({"session": spec["session"], "datetime": None})    

    return sessions

def get_session(session):
    updates = mongo.db.update.find({"session":session}, projection={'_id': False}).sort([("datetime", -1)])
    reports = mongo.db.reports.find({"session":session}, projection={'_id': False}).sort([("datetime", -1)]).limit(10)
    data = unify_reports(reports,updates)

    
    try:
        spec = get_spec(session)
        data["desc"] = spec["desc"]
    except:
        data["spec"] = None
        data["desc"] = None

    import networkx as nx

    G = nx.DiGraph()
    leaders = {}
    nodes = {}
    id = 0
    Nodes,Ids =get_associations(session)
    ids = {}
    reports = mongo.db.reports.find({"session":session}, projection={'_id': False}).sort([("datetime", -1)]).limit(1)
    reports = list(reports)
    for report in reports[0]["report"]["reports"]:
        nodes[Ids[report["source"]["id"]]] = id
        ids[id] = Ids[report["source"]["id"]]
        leaders[id] = Ids[report["leader"]]
        G.add_node(id)
        id+=1

    # d3 uses the name attribute,
    # so add a name to each node
    for n in G:
        G.nodes[n]["name"] = ids[n]
        G.add_edge(n, nodes[leaders[n]],l=0)
    # write json formatted data
    d = nx.json_graph.node_link_data(G)  # node-link format to serialize

    data["d3"] = d

    return data

def add_testbed(data):
    data = json.dumps(data, sort_keys=True)
    data = json.loads(data, object_pairs_hook=OrderedDict)
    data = SON(data)

    with mongo.cx.start_session() as mongo_session:
        with mongo_session.start_transaction():
            logging.info("New session")
            sessions = get_sessions()
            session = 0
            while session in [el["session"] for el in sessions]:
                session+=1
            item = {
                "session": session,
                "moments": [],
                "change_dates": [],
                "data": data
            }
            mongo.db.spec.replace_one({"session": session}, item, upsert=True)
    return session

def add_moment(session, data, remove_old=False):
    data = json.dumps(data, sort_keys=True)
    data = json.loads(data, object_pairs_hook=OrderedDict)
    data = SON(data)
    with mongo.cx.start_session() as mongo_session:
        with mongo_session.start_transaction():
            spec = mongo.db.spec.find_one({"session": session})
            spec["change_dates"].append(datetime.now(datetime.UTC))
            if remove_old:
                spec["moments"] = [data]
            else:
                spec["moments"].append(data)
            moment = len(spec["change_dates"])
            mongo.db.spec.replace_one({"session": session}, spec, upsert=True)
    return moment

def add_extra(session, data):
    data = json.dumps(data, sort_keys=True)
    data = json.loads(data, object_pairs_hook=OrderedDict)
    data = SON(data)
    with mongo.cx.start_session() as mongo_session:
        with mongo_session.start_transaction():
            spec = mongo.db.spec.find_one({"session": session})
            spec["extra"] = data
            mongo.db.spec.replace_one({"session": session}, spec, upsert=True)

def remove(session, all=False):
    with mongo.cx.start_session() as mongo_session:
        with mongo_session.start_transaction():
            mongo.db.update.delete_many({"session": session})
            mongo.db.reports.delete_many({"session": session})
            mongo.db.footprint.delete_many({"session": session})
            if all:
                mongo.db.spec.delete_many({"session": session})

def search_lasts(reports, update):
    # get last reports for each leader selected in the update
    selected = update["update"]["selected"]
    lasts = []
    for leader in selected:
        for report in reports:
            if report["sender"]["id"] == leader["id"]:
                lasts.append(report["report"])
                break
    return lasts


def unify_reports(reports,updates):
    # from reports and updates get the last reports for the last update
    # return both reports and update
    updates = clean_results(updates)
    logging.info("clean")
    reports = clean_results(reports)
    logging.info("clean2")
    try:
        reports = search_lasts(reports, updates[0])
        logging.info("search_lasts")
    except:
        return {"Reports":[reports[0]["report"]],"Leaders":None}
    return {"Reports":reports,"Leaders":updates[0]}

from .monitor import check_monitor

def save_report(report):
    session = report["argument"]
    item = {
        'report': report["data"],
        'session': session,
        'sender': report["sender"],
        'datetime': datetime.utcnow()
    }
    #logging.info(str(item))
    mongo.db.reports.insert_one(item)

    if check_monitor(session) is not None:
        remove_reports_older_than(session,600)


def save_update(update):
    session = update["argument"]
    item = {
        'update': update["data"],
        'session': session,
        'sender': update["sender"],
        'datetime': datetime.utcnow()
    }
    logging.info(str(item))
    mongo.db.update.insert_one(item)

    if check_monitor(session) is not None:
        remove_updates_older_than(session,600)