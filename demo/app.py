from flask import Flask, render_template, url_for, redirect, request
from dbms_communicator import DBMS_COM

app = Flask(__name__)

cities = [["New york", 100000000], ["Tokyo", 999999999], ["Mönsterås", 10]]

@app.route('/', methods=["GET", "POST"])
def index():
	if request.method == "POST":
		db.send_query(request.form['query'])
		db.fetch_result()
	return render_template('index.html', data=cities)


if (__name__) == "__main__":

	db = DBMS_COM("localhost", 8080)

	app.run(host="0.0.0.0", debug=False)
	db.close_connection()