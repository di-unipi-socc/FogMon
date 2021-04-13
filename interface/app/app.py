import os
from flask import Flask
from flask_pymongo import PyMongo
from pymongo import IndexModel, ASCENDING, DESCENDING
import logging
from views import blueprints
from model import mongo

logging.basicConfig(level=logging.INFO)

def make_app():
    app = Flask(__name__)

    app.config["MONGO_URI"] = 'mongodb://' + os.environ['MONGODB_HOSTNAME'] + ':27017/' + os.environ['MONGODB_DATABASE']

    for bp in blueprints:
        app.register_blueprint(bp)
        bp.app = app

    mongo.init_app(app)

    index_compoud = IndexModel([("datetime", DESCENDING),("session", ASCENDING)], name="session_datetime")
    index_datetime = IndexModel([("datetime", DESCENDING)], name="datetime")
    index_session = IndexModel([("session", DESCENDING)], name="session")
    index_compoud2 = IndexModel([("datetime", DESCENDING),("session", ASCENDING),("sender.id",ASCENDING)], name="session_datetime_sender")

    mongo.db.spec.create_indexes([index_session])
    mongo.db.footprint.create_indexes([index_session])
    mongo.db.reports.create_indexes([index_compoud,index_session,index_datetime,index_compoud2])
    mongo.db.update.create_indexes([index_compoud,index_session,index_datetime])
    for index in mongo.db.reports.list_indexes():
        logging.info(index)
    

    return app


if __name__ == "__main__":
    ENVIRONMENT_DEBUG = os.environ.get("APP_DEBUG", True)
    ENVIRONMENT_PORT = os.environ.get("APP_PORT", 5000)
    app = make_app()
    app.run(host='0.0.0.0', port=ENVIRONMENT_PORT, debug=ENVIRONMENT_DEBUG)