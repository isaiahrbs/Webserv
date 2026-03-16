#!/usr/bin/env python3
import os
import sys

method = os.environ.get("REQUEST_METHOD", "GET")
query  = os.environ.get("QUERY_STRING", "")

# Lire le body si POST
body_in = ""
if method == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
    if length > 0:
        body_in = sys.stdin.read(length)

print("Content-Type: text/html")
print("")
print("<html><body>")
print("<h1>CGI Hello – méthode : {}</h1>".format(method))
print("<p>QUERY_STRING : {}</p>".format(query))
if body_in:
    print("<p>Body reçu : {}</p>".format(body_in))
print("<p>SERVER_PORT : {}</p>".format(os.environ.get("SERVER_PORT", "?")))
print("<p>GATEWAY_INTERFACE : {}</p>".format(os.environ.get("GATEWAY_INTERFACE", "?")))
print("</body></html>")
