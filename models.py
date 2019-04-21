from app import db
from sqlalchemy.dialects.postgresql import JSON


class Concentrations(db.Model):
    __tablename__ = 'concentrations'

    id = db.Column(db.Integer, primary_key=True)
    location = db.Column(db.String())
    #TODO Pollutants, Humidity
    timestamp = db.Column(db.TIMESTAMP())

    def __init__(self, location, timestamp): #TODO fill Pollutants, temp, humidity
        self.location = location # Fill others like this

    def __repr__(self):
        return '<id {}>'.format(self.id)
