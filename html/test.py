#!/usr/bin/python3
import os
import sys

print("Content-Type: text/html\r\n\r")
print("<html><body>")
print("<h1>Python CGI Test</h1>")
print("<p>Environment Variables:</p>")
for key, value in os.environ.items():
    print(f"<p>{key}: {value}</p>")
print("</body></html>")