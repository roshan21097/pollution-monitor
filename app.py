from flask import Flask, render_template, request, jsonify
from flask_sqlalchemy import SQLAlchemy
import simplejson as json
import os

app = Flask(__name__)
app.config.from_object(os.environ['APP_SETTINGS'])
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

from models import Air_quality

@app.route('/')
def index():
    return render_template('home.html')

@app.route("/add")
def add_air_quality():
    location=request.args.get('location')
    co=request.args.get('co')
    so2=request.args.get('so2')
    no2=request.args.get('no2')
    pm_1_0=request.args.get('pm_1_0')
    pm_2_5=request.args.get('pm_2_5')
    pm_10=request.args.get('pm_10')
    o3=request.args.get('o3')
    t=request.args.get('t')
    wd=request.args.get('wd')
    ws=request.args.get('ws')
    h=request.args.get('h')
    p=request.args.get('p')

    try:
        air_quality=Air_quality(
            location = location,
            co = co,
            so2 = so2,
            no2 = no2,
            pm_1_0 = pm_1_0,
            pm_2_5 = pm_2_5,
            pm_10 = pm_10,
            o3 = o3,
            t = t,
            wd = wd,
            ws = ws,
            h = h,
            p = p
        )
        print(air_quality)
        db.session.add(air_quality)
        db.session.commit()
        return "Record added successfully. id={}".format(air_quality.id)
    except Exception as e:
	    return(str(e))

@app.route("/getall")
def get_all():
    try:
        air_qualitys=Air_quality.query.all()
        return  jsonify([e.serialize() for e in air_qualitys])
    except Exception as e:
	    return(str(e))

@app.route("/get/<id_>")
def get_by_id(id_):
    try:
        air_quality=Air_quality.query.filter_by(id=id_).first()
        return jsonify(air_quality.serialize())
    except Exception as e:
	    return(str(e))

if __name__ == '__main__':
    app.secret_key = "8Wy@d3E&wTin"
    app.run()
