from model import get_updates, get_spec, get_reports, mongo
from .spec import unify_reports
import logging
from statistics import mean

def compat(data):

    Leaders = data["leaders"][""]
    Leaders_list = set(val for val in Leaders.values())

    {"Reports":reports,"Leaders":updates[0]}
    data["Reports"]

def get_monitor(session):
    spec = get_spec(session)
    updates = get_updates(session)
    up = updates[0]

    if "monitor" in spec:
        if spec["monitor"] == True:
            logging.info("monitor True")
    
    updates = mongo.db.update.find({"session":session}, projection={'_id': False}).sort([("datetime", -1)])
    reports = mongo.db.reports.find({"session":session}, projection={'_id': False}).sort([("datetime", -1)]).limit(10)
    data = unify_reports(reports,updates)
    data = compat(data)
