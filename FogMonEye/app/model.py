from flask_pymongo import PyMongo
import logging
import datetime
from datetime import timedelta

mongo = PyMongo()

def deaggregate(cursor):
    vals = list(cursor)
    
    for val in vals:
        for id,v in val["_id"].items():
            val[id] = v
    return vals

def clean_results(results):
    # for result in results:
    #     result.pop("_id")
    return list(results)
    # item = {}
    # data = []
    
    # for result in results:
    #     item = {}
    #     for k,v in result.items():
    #         if k == "_id":
    #             v = str(v)
    #         item[k] = v
    #     data.append(item)
    # return data

def get_footprints(session):
    cursor = mongo.db.footprint.find({"session":session},projection={'_id': False}).sort([("moment", 1)])
    return clean_results(cursor)

def save_footprint(session, moment, data):
    item = {"session":session, "data":data, "moment": moment}
    mongo.db.footprint.replace_one({"session": session, "moment": moment}, item, upsert=True)

def get_updates(session, begin=None, end=None):
    # return updates from begin date to end date (if set)
    # in reverse order
    query = {"session": session}
    query_date = {}
    if begin is not None:
        query_date["$gt"] = begin 
    if end is not None:
        query_date["$lt"] = end 
    if begin is not None or end is not None:
        query["datetime"] = query_date
    import datetime
    def print_diff():
        nonlocal t_begin
        t_end = datetime.datetime.now()
        logging.info(f"diff updates time = {(t_end-t_begin).total_seconds()}")
        t_begin = t_end
    t_begin = datetime.datetime.now()
    cursor = mongo.db.update.find(query, projection={'_id': False}).sort([("datetime", -1)])
    print_diff()
    return clean_results(cursor)

def get_spec(session):
    cursor = mongo.db.spec.find({"session": session}, projection={'_id': False})
    return clean_results(cursor)[0]

def get_leaders(session):
    cursor = mongo.db.update.find({"session": session}, projection={'_id': False}).sort([("datetime", -1)])
    logging.info(cursor[0])
    ids = [el["id"] for el in cursor[0]["update"]["selected"]]
    return ids

def get_reports(session, ids=None, begin=None, end=None, limit=0):
    # return reports from begin date to end date (if set)
    # if ids is set then filter the id of the sender, checking if is in ids
    # in reverse order
    query = {"session": session}
    query_date = {}
    if begin is not None:
        query_date["$gt"] = begin 
    if end is not None:
        query_date["$lt"] = end 
    if begin is not None or end is not None:
        query["datetime"] = query_date
    if ids is not None:
        query["sender.id"] = {"$in": ids }
    import datetime
    def print_diff():
        nonlocal t_begin
        t_end = datetime.datetime.now()
        logging.info(f"diff reports time = {(t_end-t_begin).total_seconds()}")
        t_begin = t_end
    t_begin = datetime.datetime.now()
    cursor = mongo.db.reports.find(query, projection={'_id': False}).sort([("datetime", -1)])
    print_diff()
    #logging.info(f"exaplain get reports {session}")
    #logging.info(cursor.explain())
    if limit != 0:
        cursor.limit(limit)
    result = clean_results(cursor)
    print_diff()
    return result

def get_reports2(session, ids=None, begin=None, end=None, limit=0):
    # return reports from begin date to end date (if set)
    # if ids is set then filter the id of the sender, checking if is in ids
    # in reverse order
    query = {"session": session}
    query_date = {}
    if begin is not None:
        query_date["$gt"] = begin 
    if end is not None:
        query_date["$lt"] = end 
    if begin is not None or end is not None:
        query["datetime"] = query_date
    if ids is not None:
        query["sender.id"] = {"$in": ids }
    import datetime
    def print_diff():
        nonlocal t_begin
        t_end = datetime.datetime.now()
        logging.info(f"diff reports time = {(t_end-t_begin).total_seconds()}")
        t_begin = t_end
    t_begin = datetime.datetime.now()
    cursor = mongo.db.reports.find(query)
    print_diff()
    #logging.info(f"exaplain get reports {session}")
    #logging.info(cursor.explain())
    if limit != 0:
        cursor.limit(limit)
    logging.info(cursor.explain()["executionStats"])
    result = [doc for doc in cursor]
    print_diff()
    return result

def change_desc(session, desc):
    spec = mongo.db.spec.find_one({"session": session})
    spec["desc"] = desc
    mongo.db.spec.replace_one({"session": session}, spec, upsert=True)

def remove_reports_older_than(session,seconds, start):
    if start is None:
        start = datetime.datetime.now()
    date = start-timedelta(seconds=seconds)
    logging.info(start)
    logging.info(date)
    mongo.db.reports.delete_many({"session": session,"datetime": {"$lt": date}})

def remove_updates_older_than(session, seconds, start):
    if start is None:
        start = datetime.datetime.now()
    date = start-timedelta(seconds=seconds)
    logging.info(start)
    logging.info(date)
    mongo.db.update.delete_many({"session": session,"datetime": {"$lt": date}})