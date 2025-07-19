#!/usr/bin/env python3

import os
import sys

# Print HTTP response headers
print("Content-Type: text/html")
print()

print("<html><body>")
print("<h1>CGI Test</h1>")

# Show environment variables
print("<h2>Environment Variables</h2>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li><b>{key}</b>: {value}</li>")
print("</ul>")

# Read body from stdin
print("<h2>Request Body</h2>")
body = sys.stdin.read()
print(f"<pre>{body}</pre>")

print("</body></html>")
<<<<<<< HEAD
=======

host:   localhost:9090  1


9090

servername;  host:9090;

HOST: oma
>>>>>>> origin/omar
