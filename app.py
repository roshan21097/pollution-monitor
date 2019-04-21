from flask import Flask, render_template
from flask.ext.sqlalchemy import SQLAlchemy
import os


app = Flask(__name__)
app.config.from_object(os.environ['APP_SETTINGS'])
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

from models import Concentrations


@app.route('/')
def index():
    return render_template('home.html')


if __name__ == '__main__':
    app.secret_key = "8Wy@d3E&wTin"
    app.run()
