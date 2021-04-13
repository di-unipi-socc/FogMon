from flask import Blueprint, request, jsonify
from utils.testbed import get_sessions, get_session, save_update, save_report, add_testbed, change_testbed, remove
from utils.accuracy import accuracy
from utils.footprint import save_footprints, compute_footprint
from model import change_desc

api = Blueprint('api', __name__)

@api.route('/testbed/<int:session>/remove')
def get_convert(session):
    remove(session)
    return jsonify(
        status=True
    )

@api.route('/testbed/<int:session>/removeall')
def get_convert2(session):
    remove(session,all=True)
    return jsonify(
        status=True
    )

@api.route('/testbed')
def get_testbeds():
    data = get_sessions()
    return jsonify(
        status=True,
        data=data,
    )

@api.route('/testbed', methods=['POST'])
def post_testbed():
    data = request.get_json(force=True)
    session = add_testbed(data)

    return jsonify(
        status=True,
        message='Saved successfully!',
        session=session
    ), 201

@api.route('/testbed/<int:session>', methods=['POST'])
def put_testbed(session):
    try:
        data = request.get_json(force=True)
    except:
        import traceback
        print(traceback.format_exc(), flush=True)
        raise
    moment = change_testbed(session, data)
    return jsonify(
        status=True,
        message='Saved successfully!',
        moment=moment
    ), 201

@api.route('/testbed/<int:session>')
def get_testbed(session):
    try:
        data = get_session(session)
    except:
        data = None
        raise

    return jsonify(
        status=True,
        data=data,
    )

@api.route('/testbed/<int:session>/desc', methods=['PUT'])
def put_testbed_desc(session):
    try:
        data = request.get_json(force=True)
        desc = data["desc"]
    except:
        import traceback
        print(traceback.format_exc(), flush=True)
        raise
    change_desc(session, desc)
    return jsonify(
        status=True,
        message='Saved successfully!',
    ), 201

@api.route('/testbed/<int:session>/accuracy')
def get_accuracy(session):
    data = accuracy(session)

    return jsonify(
        status=True,
        data=data,
    )

@api.route('/testbed/<int:session>/footprint', methods=['GET'])
def get_footprint(session):
    data = compute_footprint(session)    

    return jsonify(
        status=True,
        data=data,
    )

@api.route('/testbed/<int:session>/footprint', methods=['POST'])
def post_footprint(session):
    if request.files is None:
        return jsonify(
            status=False,
            message='No files!'
        ), 400
    
    save_footprints(request.files, session)
    for file in request.files:
        print(file, flush=True)

    return jsonify(
        status=True,
        message='Saved successfully!'
    ), 201
    

@api.route('/data', methods=['POST'])
def post_data():
    data = request.get_json(force=True)
    
    try:
        if data["type"] == 0:
            save_report(data)
        elif data["type"] == 1:
            save_update(data)
    except:
        pass

    return jsonify(
        status=True,
        message='Saved successfully!'
    ), 201

@api.route('/test/<int:session>', methods=['GET'])
def test(session):
    from model import get_spec, get_updates, get_reports2,mongo,get_lastreports

    data = get_lastreports(session)
    return jsonify(
        status=True,
        message='Saved successfully!',
        data = data
    ), 201
    import logging
    spec = get_spec(session)
    spec = spec["specs"][-1]
    begin=None
    end=None
    updates = get_updates(session, begin=begin, end=end)
    # search last significant report: change!=0
    try:
        update = updates[0]
        for up in updates:
            if up["update"]["changes"] != 0:
                update = up
                break
        begin2 = update["datetime"]
    except:
        updates = get_updates(session, begin=None, end=begin)
        update = updates[0]
        begin2 = begin


    logging.info("stability:")
    logging.info(begin2)
    ids = [el["id"] for el in update["update"]["selected"]]
    reports = get_reports2(session, ids=None, begin=None, end=None)
    return jsonify(
        status=True,
        message='Saved successfully!'
    ), 201
