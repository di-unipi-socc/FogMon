from flask import Blueprint, render_template, request
from .api import get_testbeds
import logging

front = Blueprint('front', __name__)

@front.route('/')
def index():
    return render_template('index.html')

@front.route('/testbeds.html')
def testbeds():
    data = get_testbeds().json["data"]
    logging.info(data)
    return render_template('testbeds.html', data = data, num=len(data))