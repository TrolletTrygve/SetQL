from flask import Flask, render_template, url_for, redirect, request
from dbms_communicator import DBMS_COM

app = Flask(__name__)

species = ["lizard", "bear", "donkey"]

@app.route('/', methods=["GET", "POST"])
def index():
	if request.method == "POST":
		db.send_query(request.form['query'])
	return render_template('index.html', data_len=len(species), data=species)


if (__name__) == "__main__":

	db = DBMS_COM("localhost", 8080)

	app.run(host="0.0.0.0", debug=False)
	db.close_connection()