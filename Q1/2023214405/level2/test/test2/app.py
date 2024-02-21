from flask import Flask, request
import mariadb

app = Flask(__name__)
conn = mariadb.connect(
    user='root',
    password='1234',
    host= "mariadb",
    port=3306,
    database='USER')

@app.route('/register', methods=['get'])
def register():
    username = request.args.get('username')
    password = request.args.get('password')
    password2 = request.args.get('password2')
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM users WHERE username=%s", (username, ))
    user = cursor.fetchone()
    if user:
        message = '2-用户名已存在'
    elif password != password2:
        message = '2-密码不相同'
    else:
        cursor.execute("INSERT INTO users (username, password) VALUES (%s, %s)", (username, password))
        conn.commit()
        message = "2-注册成功"
    return message
        
        

@app.route('/login', methods=['GET'])
def login():
    username = request.args.get('username')
    password = request.args.get('password')
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM users WHERE username=%s AND password=%s", (username, password))
    result = cursor.fetchone()
    if result:
        message = "2-登入成功"
    else:
        message = '2-用户名或密码错误'
    return message
if __name__ == "__main__":
    app.run(host='0.0.0.0',port=5000, debug=True)
