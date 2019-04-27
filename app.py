from flask import Flask, render_template
from flask_sqlalchemy import SQLAlchemy
import os


app = Flask(__name__)

@app.route('/')
def index():
    return render_template('home.html')


if __name__ == '__main__':
    app.secret_key = "8Wy@d3E&wTin"
    app.run()
