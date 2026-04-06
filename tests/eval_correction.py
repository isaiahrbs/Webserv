#!/usr/bin/env python3
"""
eval_correction.py — Script de test calé sur la fiche de correction 42 Webserv
Couvre exactement les sections : Configuration, Basic checks, Browser, Port issues

Usage :
    python3 tests/eval_correction.py
    python3 tests/eval_correction.py --verbose

Le serveur doit tourner AVANT :
    ./webserv config/server.conf
"""

import socket
import time
import sys
import os
import threading

# ─── Ports correspondant à config/server.conf ─────────────────────────────────
HOST       = "127.0.0.1"
PORT1      = 8080   # server_name: localhost   — site statique
PORT2      = 8081   # server_name: api.localhost — CGI + upload
PORT3      = 8082   # server_name: test.localhost — redirect
TIMEOUT    = 5
VERBOSE    = "--verbose" in sys.argv or "-v" in sys.argv

# ─── Couleurs ─────────────────────────────────────────────────────────────────
GREEN  = "\033[92m"; RED   = "\033[91m"; YELLOW = "\033[93m"
CYAN   = "\033[96m"; RESET = "\033[0m";  BOLD   = "\033[1m"

passed = 0; failed = 0; errors = []

# ══════════════════════════════════════════════════════════════════════════════
# HELPERS
# ══════════════════════════════════════════════════════════════════════════════

def send_raw(host, port, raw, timeout=TIMEOUT):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(timeout)
        s.connect((host, port))
        s.sendall(raw.encode() if isinstance(raw, str) else raw)
        data = b""
        while True:
            try:
                chunk = s.recv(4096)
                if not chunk: break
                data += chunk
            except socket.timeout:
                break
        s.close()
        return _parse(data)
    except Exception as e:
        return (0, {}, f"CONNECTION_ERROR: {e}")

def _parse(raw):
    if not raw: return (0, {}, "")
    try:
        sep = raw.find(b"\r\n\r\n")
        if sep == -1: return (0, {}, raw.decode("utf-8", errors="replace"))
        hdr_raw = raw[:sep].decode("utf-8", errors="replace")
        body    = raw[sep + 4:].decode("utf-8", errors="replace")
        lines   = hdr_raw.split("\r\n")
        code    = int(lines[0].split(" ")[1]) if len(lines[0].split(" ")) >= 2 else 0
        hdrs    = {}
        for l in lines[1:]:
            if ": " in l:
                k, v = l.split(": ", 1)
                hdrs[k.lower()] = v
        return (code, hdrs, body)
    except:
        return (0, {}, raw.decode("utf-8", errors="replace"))

def check(name, cond, detail=""):
    global passed, failed
    if cond:
        passed += 1
        if VERBOSE: print(f"  {GREEN}✓{RESET} {name}")
    else:
        failed += 1
        msg = f"  {RED}✗{RESET} {name}"
        if detail: msg += f"\n    {YELLOW}→ {detail}{RESET}"
        print(msg)
        errors.append(name)

def section(num, title, pdf_ref=""):
    tag = f"  {YELLOW}[PDF: {pdf_ref}]{RESET}" if pdf_ref else ""
    print(f"\n{BOLD}{CYAN}{'━'*58}")
    print(f"  {num}. {title}{tag}")
    print(f"{'━'*58}{RESET}")

def make_file(path, content="test content"):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f: f.write(content)

# ══════════════════════════════════════════════════════════════════════════════
# SECTION 1 — CONFIGURATION (PDF page 3)
# ══════════════════════════════════════════════════════════════════════════════

def test_config_multiple_ports():
    section("1", "Plusieurs serveurs sur des ports différents",
            "Configuration → multiple servers with different port")

    for port, name in [(PORT1, "8080"), (PORT2, "8081"), (PORT3, "8082")]:
        code, _, _ = send_raw(HOST, port,
            f"GET / HTTP/1.1\r\nHost: {HOST}:{port}\r\nConnection: close\r\n\r\n")
        check(f"Port {name} répond (code != 0)", code != 0, f"code={code}")

def test_config_virtual_hosting():
    section("2", "Virtual hosting par hostname (Host: header)",
            "Configuration → multiple servers with different hostname")

    # Equivalent de : curl --resolve api.localhost:8081:127.0.0.1 http://api.localhost:8081/
    # On envoie le Host: header pour router vers le bon serveur

    # Server1 : Host: localhost → doit servir www/website
    code1, _, body1 = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Host: localhost → 200", code1 == 200, f"got {code1}")

    # Server2 : Host: api.localhost → route /scripts connue
    code2, _, _ = send_raw(HOST, PORT2,
        "GET /scripts/hello.py HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
    check("Host: api.localhost → CGI exécuté (200)", code2 == 200, f"got {code2}")

    # Server3 : Host: test.localhost → doit servir index.html
    code3, _, _ = send_raw(HOST, PORT3,
        "GET / HTTP/1.1\r\nHost: test.localhost\r\nConnection: close\r\n\r\n")
    check("Host: test.localhost → 200", code3 == 200, f"got {code3}")

    # Mauvais hostname sur un port → fallback sur le premier serveur du port (comportement attendu)
    code_fb, _, _ = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: unknown.host\r\nConnection: close\r\n\r\n")
    check("Host inconnu → fallback (200, pas de crash)", code_fb in (200, 404), f"got {code_fb}")

def test_config_error_pages():
    section("3", "Pages d'erreur personnalisées",
            "Configuration → setup default error page (404)")

    code, hdrs, body = send_raw(HOST, PORT1,
        "GET /page_qui_nexiste_pas HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("404 retourné", code == 404, f"got {code}")
    check("Content-Type: text/html sur erreur 404",
          "text/html" in hdrs.get("content-type", ""), hdrs.get("content-type"))
    check("Body 404 non vide", len(body) > 0, "body vide")

    # 405 sur route qui n'autorise pas DELETE
    code, _, _ = send_raw(HOST, PORT1,
        "DELETE / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Méthode non autorisée → 405", code == 405, f"got {code}")

def test_config_body_limit():
    section("4", "Limitation taille du body (max_body_size)",
            "Configuration → limit the client body")

    # Server3 : max_body_size = 100 000 → envoyer 200 Ko
    # curl -X POST -H "Content-Type: plain/text" --data "BODY..."
    big = "X" * 200_000
    req = (
        f"POST / HTTP/1.1\r\nHost: test.localhost\r\n"
        f"Content-Length: {len(big)}\r\nContent-Type: text/plain\r\n"
        f"Connection: close\r\n\r\n" + big
    )
    code, _, _ = send_raw(HOST, PORT3, req, timeout=10)
    check("Body > max_body_size (server3 100KB) → 413 ou 405",
          code in (413, 405), f"got {code}")

    # Server2 : max_body_size = 5 000 000 → envoyer 6 Mo
    big2 = "Y" * 6_000_000
    req2 = (
        f"POST /upload/oversized.bin HTTP/1.1\r\nHost: api.localhost\r\n"
        f"Content-Length: {len(big2)}\r\nContent-Type: application/octet-stream\r\n"
        f"Connection: close\r\n\r\n" + big2
    )
    code2, _, _ = send_raw(HOST, PORT2, req2, timeout=15)
    check("Body > 5MB (server2) → 413", code2 == 413, f"got {code2}")

def test_config_routes():
    section("5", "Routes vers différents répertoires",
            "Configuration → setup routes in a server to different directories")

    # /scripts → www/server2/scripts
    code, _, body = send_raw(HOST, PORT2,
        "GET /scripts/hello.py HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
    check("Route /scripts → CGI Python exécuté", code == 200, f"got {code}")

    # / → www/website (server1)
    code, _, body = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Route / → fichier index servi (200)", code == 200, f"got {code}")
    check("Contenu HTML retourné", "html" in body.lower(), body[:100])

def test_config_default_index():
    section("6", "Fichier index par défaut sur répertoire",
            "Configuration → setup a default file to search for in a directory")

    # GET / sur server1 → doit servir website.html (configuré comme index)
    code, hdrs, body = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("GET / → index servi automatiquement (200)", code == 200, f"got {code}")
    check("Réponse contient du HTML", "html" in body.lower(), body[:80])

def test_config_method_restriction():
    section("7", "Restriction des méthodes par route",
            "Configuration → setup accepted methods (try delete with/without permission)")

    # /upload → POST uniquement → GET doit être refusé (405)
    code, _, _ = send_raw(HOST, PORT2,
        "GET /upload/ HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
    check("GET sur route POST-only → 405", code == 405, f"got {code}")

    # /files → GET POST DELETE autorisés → DELETE doit marcher
    make_file("www/server2/files/perm_test.txt", "permission test")
    code, _, _ = send_raw(HOST, PORT2,
        "DELETE /files/perm_test.txt HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
    check("DELETE sur route qui l'autorise → 204", code == 204, f"got {code}")

    # / sur server1 → GET only → DELETE refusé
    code, _, _ = send_raw(HOST, PORT1,
        "DELETE / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("DELETE sur route GET-only → 405", code == 405, f"got {code}")

# ══════════════════════════════════════════════════════════════════════════════
# SECTION 2 — BASIC CHECKS (PDF page 3)
# ══════════════════════════════════════════════════════════════════════════════

def test_basic_get():
    section("8", "Requêtes GET", "Basic checks → GET requests")

    code, hdrs, body = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("GET / → 200", code == 200, f"got {code}")
    check("Content-Type présent", "content-type" in hdrs, str(hdrs))
    check("Content-Length présent", "content-length" in hdrs, str(hdrs))
    check("Body non vide", len(body) > 0)

    # Fichier inexistant
    code, _, _ = send_raw(HOST, PORT1,
        "GET /fichier_inexistant.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("GET fichier inexistant → 404", code == 404, f"got {code}")

def test_basic_post():
    section("9", "Requêtes POST", "Basic checks → POST requests")

    # Upload d'un fichier texte
    body_data = "contenu de test pour l'upload POST"
    req = (
        f"POST /upload/eval_test.txt HTTP/1.1\r\nHost: api.localhost\r\n"
        f"Content-Length: {len(body_data)}\r\nContent-Type: text/plain\r\n"
        f"Connection: close\r\n\r\n{body_data}"
    )
    code, _, _ = send_raw(HOST, PORT2, req)
    check("POST upload → 201", code == 201, f"got {code}")
    check("Fichier présent sur disque après upload",
          os.path.exists("www/server2/uploads/eval_test.txt"))

def test_basic_delete():
    section("10", "Requêtes DELETE", "Basic checks → DELETE requests")

    # Créer un fichier à supprimer
    make_file("www/server2/files/to_delete.txt", "à supprimer")

    code, _, _ = send_raw(HOST, PORT2,
        "DELETE /files/to_delete.txt HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
    check("DELETE fichier existant → 204", code == 204, f"got {code}")
    check("Fichier bien supprimé du disque",
          not os.path.exists("www/server2/files/to_delete.txt"))

    # DELETE fichier inexistant → 404
    code, _, _ = send_raw(HOST, PORT2,
        "DELETE /files/fantome.txt HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
    check("DELETE fichier inexistant → 404", code == 404, f"got {code}")

def test_basic_unknown_method():
    section("11", "Méthode inconnue (pas de crash)",
            "Basic checks → UNKNOWN requests should not crash")

    code, _, _ = send_raw(HOST, PORT1,
        "FOOBAR / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Méthode FOOBAR → 400 ou 405 (pas de crash)", code in (400, 405), f"got {code}")

    # Vérifier que le serveur est toujours en vie après la méthode inconnue
    code2, _, _ = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Serveur toujours vivant après méthode inconnue", code2 == 200, f"got {code2}")

def test_basic_upload_and_retrieve():
    section("12", "Upload d'un fichier puis récupération",
            "Basic checks → upload some file to the server and get it back")

    content = "Hello 42 Lausanne! Fichier de test upload/download."
    filename = "upload_retrieve_test.txt"
    upload_path = f"www/server2/uploads/{filename}"

    # 1. Upload via POST
    req = (
        f"POST /upload/{filename} HTTP/1.1\r\nHost: api.localhost\r\n"
        f"Content-Length: {len(content)}\r\nContent-Type: text/plain\r\n"
        f"Connection: close\r\n\r\n{content}"
    )
    code, _, _ = send_raw(HOST, PORT2, req)
    check("Upload POST → 201", code == 201, f"got {code}")

    # 2. Vérifier présence sur disque
    file_ok = os.path.exists(upload_path)
    check("Fichier présent sur le disque", file_ok)

    # 3. Récupérer via GET (route /files lit www/server2/files — on copie le fichier)
    if file_ok:
        import shutil
        dest = "www/server2/files/upload_retrieve_test.txt"
        shutil.copy(upload_path, dest)
        code2, hdrs2, body2 = send_raw(HOST, PORT2,
            f"GET /files/{filename} HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
        check("GET du fichier uploadé → 200", code2 == 200, f"got {code2}")
        check("Contenu du fichier intact", content in body2, body2[:100])
        if os.path.exists(dest): os.remove(dest)

# ══════════════════════════════════════════════════════════════════════════════
# SECTION 3 — BROWSER CHECKS (PDF page 4)
# ══════════════════════════════════════════════════════════════════════════════

def test_browser_static_website():
    section("13", "Site statique complet (compatible navigateur)",
            "Browser → serve a fully static website")

    code, hdrs, body = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Site statique → 200", code == 200, f"got {code}")
    check("Content-Type: text/html", "text/html" in hdrs.get("content-type",""),
          hdrs.get("content-type"))
    check("Réponse contient <html>", "<html" in body.lower() or "<!doctype" in body.lower(),
          body[:120])
    check("Content-Length correct",
          "content-length" in hdrs and
          int(hdrs["content-length"]) == len(body.encode("utf-8", errors="replace")),
          f"cl={hdrs.get('content-length')} / body={len(body.encode())}")

def test_browser_wrong_url():
    section("14", "URL incorrecte → 404", "Browser → try a wrong URL")

    for path in ["/does_not_exist", "/foo/bar/baz", "/index.php"]:
        code, _, body = send_raw(HOST, PORT1,
            f"GET {path} HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
        check(f"GET {path} → 404", code == 404, f"got {code}")

def test_browser_directory_listing():
    section("15", "Listing de répertoire (autoindex)",
            "Browser → try to list a directory")

    # server3 n'a pas autoindex, server1 non plus sur /
    # On teste qu'un répertoire sans index retourne 403 ou un listing
    code, hdrs, body = send_raw(HOST, PORT3,
        "GET / HTTP/1.1\r\nHost: test.localhost\r\nConnection: close\r\n\r\n")
    # server3 a un index.html → 200 attendu
    check("GET répertoire avec index → 200", code == 200, f"got {code}")

    # Tester un sous-dossier sans index sur server3
    code2, _, body2 = send_raw(HOST, PORT3,
        "GET /new/ HTTP/1.1\r\nHost: test.localhost\r\nConnection: close\r\n\r\n")
    check("GET /new/ → 200 ou 403 (pas de crash)", code2 in (200, 403, 404), f"got {code2}")
    if code2 == 200 and "directory" in body2.lower():
        check("Listing HTML contient des entrées", "<a " in body2.lower(), body2[:200])

def test_browser_redirect():
    section("16", "Redirection HTTP 301",
            "Browser → try a redirected URL")

    code, hdrs, _ = send_raw(HOST, PORT3,
        "GET /old HTTP/1.1\r\nHost: test.localhost\r\nConnection: close\r\n\r\n")
    check("GET /old → 301 Moved Permanently", code == 301, f"got {code}")
    check("Header Location présent", "location" in hdrs, str(hdrs))
    loc = hdrs.get("location", "")
    check("Location pointe vers /new", "/new" in loc, f"Location: {loc}")

def test_browser_cgi():
    section("17", "Exécution CGI Python",
            "Browser → CGI")

    # GET CGI
    code, hdrs, body = send_raw(HOST, PORT2,
        "GET /scripts/hello.py HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
    check("CGI GET → 200", code == 200, f"got {code}")
    check("CGI retourne HTML", "<html" in body.lower(), body[:200])
    check("GATEWAY_INTERFACE présent dans output",
          "CGI/1.1" in body, body[:300])

    # GET CGI avec query string
    code2, _, body2 = send_raw(HOST, PORT2,
        "GET /scripts/hello.py?name=eval&value=42 HTTP/1.1\r\nHost: api.localhost\r\nConnection: close\r\n\r\n")
    check("CGI query string → 200", code2 == 200, f"got {code2}")
    check("QUERY_STRING transmis", "name=eval" in body2 or "value=42" in body2, body2[:300])

    # POST CGI
    post_body = "data=hello_world"
    req = (
        f"POST /scripts/hello.py HTTP/1.1\r\nHost: api.localhost\r\n"
        f"Content-Length: {len(post_body)}\r\nContent-Type: application/x-www-form-urlencoded\r\n"
        f"Connection: close\r\n\r\n{post_body}"
    )
    code3, _, body3 = send_raw(HOST, PORT2, req)
    check("CGI POST → 200", code3 == 200, f"got {code3}")
    check("CGI affiche méthode POST", "POST" in body3, body3[:300])

# ══════════════════════════════════════════════════════════════════════════════
# SECTION 4 — PORT ISSUES (PDF page 4)
# ══════════════════════════════════════════════════════════════════════════════

def test_port_multiple_sites():
    section("18", "Plusieurs ports = plusieurs sites différents",
            "Port issues → multiple ports, different websites")

    code1, _, body1 = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    code3, _, body3 = send_raw(HOST, PORT3,
        "GET / HTTP/1.1\r\nHost: test.localhost\r\nConnection: close\r\n\r\n")

    check("Port 8080 répond avec du contenu", code1 == 200 and len(body1) > 0,
          f"code={code1}")
    check("Port 8082 répond avec du contenu", code3 == 200 and len(body3) > 0,
          f"code={code3}")
    check("Les deux ports servent des contenus distincts",
          body1 != body3, "contenu identique sur les deux ports")

def test_port_duplicate_handling():
    section("19", "Port dupliqué dans la config — géré proprement",
            "Port issues → same port multiple times should not work")

    # On ne peut pas recharger la config en live, mais on vérifie que le serveur
    # actuel tourne sans crash sur les 3 ports. Le sujet dit que la config
    # avec port dupliqué ne doit pas planter le serveur (il doit soit refuser,
    # soit ne créer qu'un seul listener).

    all_ok = True
    for port in [PORT1, PORT2, PORT3]:
        code, _, _ = send_raw(HOST, port,
            f"GET / HTTP/1.1\r\nHost: {HOST}:{port}\r\nConnection: close\r\n\r\n")
        if code == 0:
            all_ok = False
    check("Les 3 ports répondent (config valide sans duplication)", all_ok)
    print(f"  {YELLOW}ℹ  Note : tester une config avec port dupliqué manuellement :{RESET}")
    print(f"       Ajouter deux blocs 'server' avec le même port dans server.conf")
    print(f"       Le serveur doit démarrer en ignorant le doublon (log 'already listened')")

# ══════════════════════════════════════════════════════════════════════════════
# SECTION 5 — ROBUSTESSE (pas dans le PDF mais testé pendant la défense)
# ══════════════════════════════════════════════════════════════════════════════

def test_robustness():
    section("20", "Robustesse — le serveur ne crash pas",
            "Guidelines → no segfault, no unexpected termination")

    # Déconnexion brutale
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT1))
        s.close()
        time.sleep(0.1)
    except: pass
    code, _, _ = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Survit à déconnexion brutale", code == 200, f"got {code}")

    # Données binaires aléatoires
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2)
        s.connect((HOST, PORT1))
        s.sendall(b"\x00\xff\xfe\xfd garbage \r\n !@#$%^&*()")
        try: s.recv(4096)
        except: pass
        s.close()
        time.sleep(0.1)
    except: pass
    code2, _, _ = send_raw(HOST, PORT1,
        "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Survit à données binaires invalides", code2 == 200, f"got {code2}")

    # Directory traversal
    code3, _, body3 = send_raw(HOST, PORT1,
        "GET /../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Directory traversal → pas de /etc/passwd dans le body",
          "root:" not in body3 and code3 in (400, 403, 404),
          f"code={code3}")

    # Version HTTP invalide
    code4, _, _ = send_raw(HOST, PORT1,
        "GET / HTTP/9.9\r\nHost: localhost\r\nConnection: close\r\n\r\n")
    check("Version HTTP invalide → 400 (pas de crash)", code4 == 400, f"got {code4}")

    # Connexions simultanées
    results = []
    def worker(i):
        c, _, _ = send_raw(HOST, PORT1,
            f"GET / HTTP/1.1\r\nHost: localhost\r\nX-Client: {i}\r\nConnection: close\r\n\r\n")
        results.append(c)
    threads = [threading.Thread(target=worker, args=(i,)) for i in range(20)]
    for t in threads: t.start()
    for t in threads: t.join(timeout=10)
    ok = sum(1 for c in results if c == 200)
    check(f"20 connexions simultanées : {ok}/20 réussies", ok >= 18,
          f"{20 - ok} échecs")

def test_chunked_encoding():
    section("21", "Chunked Transfer Encoding",
            "Basic checks → POST chunked")

    chunk   = b"Chunked upload test 42!"
    hex_sz  = hex(len(chunk))[2:]
    payload = (
        b"POST /upload/chunked_eval.txt HTTP/1.1\r\n"
        b"Host: api.localhost\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: text/plain\r\n"
        b"Connection: close\r\n\r\n"
        + hex_sz.encode() + b"\r\n"
        + chunk + b"\r\n"
        + b"0\r\n\r\n"
    )
    code, _, _ = send_raw(HOST, PORT2, payload)
    check("POST chunked → 201", code == 201, f"got {code}")

# ══════════════════════════════════════════════════════════════════════════════
# MAIN
# ══════════════════════════════════════════════════════════════════════════════

if __name__ == "__main__":
    # Se placer à la racine du projet
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
    os.chdir(root)

    print(f"\n{BOLD}{'═'*58}")
    print(f"  WEBSERV — SCRIPT DE CORRECTION 42 LAUSANNE")
    print(f"{'═'*58}{RESET}")
    print(f"  Serveur attendu sur 127.0.0.1:{PORT1}/{PORT2}/{PORT3}")
    print(f"  Mode verbose : {'oui' if VERBOSE else 'non  (--verbose pour détails)'}\n")

    # Vérifier que le serveur tourne
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2)
        s.connect((HOST, PORT1))
        s.close()
    except Exception:
        print(f"{RED}ERREUR : Impossible de joindre {HOST}:{PORT1}")
        print(f"Lance le serveur d'abord :{RESET}")
        print(f"    ./webserv config/server.conf\n")
        sys.exit(1)

    t0 = time.time()

    # ── Mandatory Part ───────────────────────────────────────────────────────
    print(f"{BOLD}── CONFIGURATION ───────────────────────────────────────{RESET}")
    test_config_multiple_ports()
    test_config_virtual_hosting()
    test_config_error_pages()
    test_config_body_limit()
    test_config_routes()
    test_config_default_index()
    test_config_method_restriction()

    print(f"\n{BOLD}── BASIC CHECKS ────────────────────────────────────────{RESET}")
    test_basic_get()
    test_basic_post()
    test_basic_delete()
    test_basic_unknown_method()
    test_basic_upload_and_retrieve()

    print(f"\n{BOLD}── BROWSER CHECKS ──────────────────────────────────────{RESET}")
    test_browser_static_website()
    test_browser_wrong_url()
    test_browser_directory_listing()
    test_browser_redirect()
    test_browser_cgi()

    print(f"\n{BOLD}── PORT ISSUES ─────────────────────────────────────────{RESET}")
    test_port_multiple_sites()
    test_port_duplicate_handling()

    print(f"\n{BOLD}── ROBUSTESSE ──────────────────────────────────────────{RESET}")
    test_robustness()
    test_chunked_encoding()

    # ── Résultats ────────────────────────────────────────────────────────────
    elapsed = time.time() - t0
    total   = passed + failed

    print(f"\n{BOLD}{'═'*58}")
    if failed == 0:
        print(f"  {GREEN}RÉSULTATS : {passed}/{total} tests passés ✓  [{elapsed:.2f}s]{RESET}")
    else:
        print(f"  RÉSULTATS : {GREEN}{passed}{RESET}/{total} passés  "
              f"{RED}{failed} échoués{RESET}  [{elapsed:.2f}s]")
    print(f"{BOLD}{'═'*58}{RESET}\n")

    if failed > 0:
        print(f"{RED}Tests échoués :{RESET}")
        for e in errors:
            print(f"  {RED}✗{RESET} {e}")
        print()

    print(f"{YELLOW}ℹ  Siege stress test (à faire manuellement) :{RESET}")
    print(f"    siege -b -c 10 -t 30s http://127.0.0.1:{PORT1}/")
    print(f"    → Availability doit être > 99.5%\n")

    sys.exit(0 if failed == 0 else 1)
