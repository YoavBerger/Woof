from flask import Flask, request, render_template_string
import sqlite3

app = Flask(__name__)

def init_db():
    conn = sqlite3.connect('database.db')
    cursor = conn.cursor()
    cursor.execute('''CREATE TABLE IF NOT EXISTS users
                      (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, password TEXT)''')
    cursor.execute('''CREATE TABLE IF NOT EXISTS messages
                      (id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER, message TEXT)''')
    conn.commit()
    conn.close()

@app.route('/')
def home():
    return render_template_string('''
        <h1>Welcome</h1>
        <a href="/register">Register</a><br>
        <a href="/login">Login</a><br>
        <a href="/forum">Forum</a><br>
    ''')

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        conn = sqlite3.connect('database.db')
        cursor = conn.cursor()
        cursor.execute(f"INSERT INTO users (username, password) VALUES ('{username}', '{password}')")
        conn.commit()
        conn.close()
        return 'Registered successfully!'

    return render_template_string('''
        <form method="post">
            Username: <input type="text" name="username"><br>
            Password: <input type="password" name="password"><br>
            <input type="submit" value="Register">
        </form>
    ''')

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        conn = sqlite3.connect('database.db')
        cursor = conn.cursor()
        cursor.execute(f"SELECT * FROM users WHERE username='{username}' AND password='{password}'")
        user = cursor.fetchone()
        conn.close()
        if user:
            return 'Logged in successfully!'
        else:
            return 'Invalid credentials!'

    return render_template_string('''
        <form method="post">
            Username: <input type="text" name="username"><br>
            Password: <input type="password" name="password"><br>
            <input type="submit" value="Login">
        </form>
    ''')

@app.route('/forum', methods=['GET', 'POST'])
def forum():
    if request.method == 'POST':
        message = request.form['message']
        user_id = 1  # Assume user ID is 1 for simplicity
        conn = sqlite3.connect('database.db')
        cursor = conn.cursor()
        cursor.execute(f"INSERT INTO messages (user_id, message) VALUES ('{user_id}', '{message}')")
        conn.commit()
        conn.close()

    conn = sqlite3.connect('database.db')
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM messages")
    messages = cursor.fetchall()
    conn.close()

    messages_html = ''.join(f'<p>{message[2]}</p>' for message in messages)

    return render_template_string('''
        <h1>Forum</h1>
        <form method="post">
            Message: <textarea name="message"></textarea><br>
            <input type="submit" value="Post">
        </form>
        <h2>Messages</h2>
        {}
        <h2>Search Messages</h2>
        <form action="/search" method="get">
            <input type="text" name="query" placeholder="Search"><br>
            <input type="submit" value="Search">
        </form>
    '''.format(messages_html))

@app.route('/search', methods=['GET'])
def search():
    query = request.args.get('query', '')
    conn = sqlite3.connect('database.db')
    cursor = conn.cursor()
    cursor.execute(f"SELECT * FROM messages WHERE message LIKE '%{query}%'")
    messages = cursor.fetchall()
    conn.close()

    messages_html = ''.join(f'<p>{message[2]}</p>' for message in messages)

    return render_template_string(f'''
        <h1>Search Results</h1>
        <form action="/search" method="get">
            <input type="text" name="query" value="{query}" placeholder="Search"><br>
            <input type="submit" value="Search">
        </form>
        <h2>Messages</h2>
        {messages_html}
    ''')

if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=3000, debug=True)
