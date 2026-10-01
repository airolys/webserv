import os
import cgi
import http.cookies
from datetime import datetime, timedelta

print("Content-Type: text/html", end="\r\n")

cookie = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE"))
username = cookie.get("username")

if username:
    print(f"\r\n<h1>Welcome back, {username.value}!</h1>")
else:
    form = cgi.FieldStorage()
    if "username" in form:
        username = form.getvalue("username")
        expires = (datetime.utcnow() + timedelta(minutes=1)).strftime("%a, %d-%b-%Y %H:%M:%S GMT")
        cookie = http.cookies.SimpleCookie()
        cookie["username"] = username
        cookie["username"]["path"] = "/"
        cookie["username"]["expires"] = expires
        print(cookie.output())
        print("\r\n\r\n")
        print(f"<h1>Welcome, {username}!</h1>")
    else:
        print("\r\n")
        print('''<form method="POST" action="">
                <label for="username">Enter your name:</label>
                <input type="text" id="username" name="username" required>
                <input type="submit" value="Submit">
              </form>''')
