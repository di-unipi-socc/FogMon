from flask_pymongo import PyMongo
import logging

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

def get_lastreports(session):
    import datetime
    # date1 = datetime.datetime.strptime("2021-03-10 16:09:20.918000", '%Y-%m-%d %H:%M:%S.%f')
    # date2 = datetime.datetime.strptime("2021-03-10 16:16:52.135000", '%Y-%m-%d %H:%M:%S.%f')
    
    # mongo.db.reports.remove({"session": session,"datetime": {"$lt": date2, "$gt": date1}})
    #mongo.db.update.remove({"session": session,"update.selected": {"$eq": []}})
    
    spec = get_spec(session)
    if session == 113:
        pass
        #mongo.db.update.remove({"session":113,"datetime": datetime.datetime(2021, 3, 21, 0, 18, 16, 340000)})
        #spec["change_dates"] = [datetime.datetime.strptime("2021-03-25 17:47:26.918000", '%Y-%m-%d %H:%M:%S.%f')]
        #mongo.db.spec.replace_one({"session": session}, spec, upsert=True)

    return spec
    cursor = mongo.db.update.find({"session": session}).sort([("datetime", 1)])

    cursor = mongo.db.update.find({"session": session}).sort([("datetime", 1)])
    updates = list(cursor)
    date = updates[0]["datetime"]
    cursor = mongo.db.reports.find({"session": session,"datetime": {"$lt": date}}).sort([("datetime", 1)])
    els = list(cursor)
    ids2 = [(len(el["report"]["reports"]),el["sender"]["id"],el["datetime"]) for el in els ]
    for el in updates:
        date = el["datetime"]
        logging.info(date)
    logging.info("\n\n\n")
    logging.info(ids2)
    logging.info(date)
    for el in updates[1:]:
        first_date = date
        date = el["datetime"]
        cursor = mongo.db.reports.find({"session": session,"datetime": {"$lt": date, "$gt": first_date}}).sort([("datetime", 1)])
        els = list(cursor)
        ids2 = [(len(el["report"]["reports"]),el["sender"]["id"],el["datetime"]) for el in els ]
        logging.info(ids2)
        logging.info(date)
    cursor = mongo.db.reports.find({"session": session,"datetime": {"$gt": date}}).sort([("datetime", 1)])
    els = list(cursor)
    ids2 = [(len(el["report"]["reports"]),el["sender"]["id"],el["datetime"]) for el in els ]
    logging.info(ids2)

    ids = [el["id"] for el in updates[0]["update"]["selected"]]
    logging.info(ids)
    cursor = mongo.db.reports.find({"session": session, "sender.id": {"$in": ids }, "datetime": {"$gt": date}}).sort([("datetime", -1)])
    els = list(cursor)
    logging.info(len(els))
    ids2 = [el["sender"]["id"] for el in els]
    logging.info(ids2)
    reports = []
    checked = []
    for report in els:
        id = report["sender"]["id"]
        if id in ids and id not in checked:
            reports.append(report)
            checked.append(id)
    reports = clean_results(reports)
    return reports
