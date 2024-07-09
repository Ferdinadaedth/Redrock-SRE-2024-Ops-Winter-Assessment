from flask import Flask, request, render_template

import mysql.connector

# 连接数据库

mydatabase = mysql.connector.connect(
    host="192.168.201.138",
    user="root",
    password="root",
    database="level1"
)
# 创建游标对象
mycursor = mydatabase.cursor()

app = Flask(__name__)


@app.route('/')
def hello_world():  # put application's code here
    return 'Hello World!'


@app.route('/login/')
def login():
    page = request.args.get(key='page', default=1, type=int)
    return render_template("login.html")


@app.route('/login_result/', methods=["GET", "POST"])
def login_result():
    if request.method == "POST":
        values = request.form
        username = request.form['username']
        email = request.form['email']
        sql = f"insert into user (username,email) values (%s,%s)"
        mycursor.execute(sql, (username, email))
        mydatabase.commit()
        return render_template('login_result.html', values=values)


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
