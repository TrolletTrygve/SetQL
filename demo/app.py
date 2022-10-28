from flask import Flask, render_template, url_for, redirect, request
from dbms_communicator import DBMS_COM

app = Flask(__name__)

count = 3
city_data = [["New york", "Umeå", "Mönsterås"], [1000000, 50, 10]]

@app.route('/', methods=["GET", "POST"])
def index():
	global city_data
	global count

	if request.method == "POST":
		q = request.form['query']

		if (not q == ""):
			db.send_query(request.form['query'])

			print("Getting results")
			city_data = db.fetch_result()

			count = 0
			try:
				count = len(city_data[0])
			except:
				print("No data was retrieved")

	return render_template('index.html', data=city_data, count=count)


if (__name__) == "__main__":

	db = DBMS_COM("localhost", 8080)

	app.run(host="0.0.0.0", debug=False)
	db.close_connection()