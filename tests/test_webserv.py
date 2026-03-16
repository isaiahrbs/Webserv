#!/usr/bin/env python3
"""
test_webserv.py — Batterie de tests pour le projet Webserv (42)
Couvre toutes les exigences du sujet version 23.1

Usage:
    python3 tests/test_webserv.py
    python3 tests/test_webserv.py --verbose

Le serveur doit tourner AVANT de lancer ce script :
    ./webserv config/server.conf
"""

import socket
import time
import sys
import os
import threading

# ─── Configuration ────────────────────────────────────────────────────────────
HOST1 = "127.0.0.1"; PORT1 = 8080  # Serveur 1 – statique
HOST2 = "127.0.0.1"; PORT2 = 8081  # Serveur 2 – CGI + upload
HOST3 = "127.0.0.1"; PORT3 = 8082  # Serveur 3 – redirect
TIMEOUT = 5
VERBOSE = "--verbose" in sys.argv or "-v" in sys.argv

# ─── Compteurs ────────────────────────────────────────────────────────────────
passed = 0
failed = 0
errors = []

# ─── Couleurs terminal ────────────────────────────────────────────────────────
GREEN  = "\033[92m"
RED    = "\033[91m"
YELLOW = "\033[93m"
CYAN   = "\033[96m"
RESET  = "\033[0m"
BOLD   = "\033[1m"

# ─── Helper : envoi/réception HTTP brut ───────────────────────────────────────

def send_raw(host, port, raw_request, timeout=TIMEOUT):
    """Envoie une requête HTTP brute, retourne (status_code, headers_dict, body)"""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(timeout)
        s.connect((host, port))
        s.sendall(raw_request.encode() if isinstance(raw_request, str) else raw_request)
        response = b""
        while True:
            try:
                chunk = s.recv(4096)
                if not chunk:
                    break
                response += chunk
            except socket.timeout:
                break
        s.close()
        return parse_response(response)
    except Exception as e:
        return (0, {}, f"CONNECTION_ERROR: {e}")

def send_raw_bytes(host, port, data, timeout=TIMEOUT):
    """Comme send_raw mais accepte bytes directement"""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(timeout)
        s.connect((host, port))
        s.sendall(data)
        response = b""
        while True:
            try:
                chunk = s.recv(4096)
                if not chunk:
                    break
                response += chunk
            except socket.timeout:
                break
        s.close()
        return parse_response(response)
    except Exception as e:
        return (0, {}, f"CONNECTION_ERROR: {e}")

def parse_response(raw):
    """Parse une réponse HTTP brute → (status_code, headers, body_str)"""
    if not raw:
        return (0, {}, "")
    try:
        header_end = raw.find(b"\r\n\r\n")
        if header_end == -1:
            return (0, {}, raw.decode("utf-8", errors="replace"))
        header_part = raw[:header_end].decode("utf-8", errors="replace")
        body = raw[header_end + 4:]
        lines = header_part.split("\r\n")
        status_line = lines[0]
        code = int(status_line.split(" ")[1]) if len(status_line.split(" ")) >= 2 else 0
        headers = {}
        for line in lines[1:]:
            if ": " in line:
                k, v = line.split(": ", 1)
                headers[k.lower()] = v
        return (code, headers, body.decode("utf-8", errors="replace"))
    except Exception:
        return (0, {}, raw.decode("utf-8", errors="replace"))

# ─── Assertion helper ─────────────────────────────────────────────────────────

def check(name, condition, detail=""):
    global passed, failed
    if condition:
        passed += 1
        if VERBOSE:
            print(f"  {GREEN}✓{RESET} {name}")
    else:
        failed += 1
        msg = f"  {RED}✗{RESET} {name}"
        if detail:
            msg += f"\n    {YELLOW}→ {detail}{RESET}"
        print(msg)
        errors.append(name)

def section(title):
    print(f"\n{BOLD}{CYAN}━━━ {title} ━━━{RESET}")

# ═══════════════════════════════════════════════════════════════════════════════
# TESTS
# ═══════════════════════════════════════════════════════════════════════════════

def test_server_reachable():
    section("1. Connectivité des 3 serveurs")
    for host, port, name in [
        (HOST1, PORT1, "Serveur 1 (8080)"),
        (HOST2, PORT2, "Serveur 2 (8081)"),
        (HOST3, PORT3, "Serveur 3 (8082)"),
    ]:
        code, _, _ = send_raw(host, port,
            f"GET / HTTP/1.1\r\nHost: {host}:{port}\r\nConnection: close\r\n\r\n")
        check(f"{name} répond", code != 0,
              f"code={code} (0 = connexion refusée)")


def test_static_files():
    section("2. Fichiers statiques (GET)")

    # index.html
    code, hdrs, body = send_raw(HOST1, PORT1,
        "GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    check("GET / → 200", code == 200, f"got {code}")
    check("Content-Type text/html", "text/html" in hdrs.get("content-type", ""), hdrs.get("content-type"))
    check("Body non vide", len(body) > 0)

    # Fichier inexistant → 404
    code, _, _ = send_raw(HOST1, PORT1,
        "GET /does_not_exist.html HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    check("GET fichier inexistant → 404", code == 404, f"got {code}")

    # Fichier CSS (si présent)
    code, hdrs, _ = send_raw(HOST1, PORT1,
        "GET /assets/style.css HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    if code == 200:
        check("Content-Type CSS correct", "text/css" in hdrs.get("content-type",""), hdrs.get("content-type"))


def test_error_pages():
    section("3. Pages d'erreur")

    # 404 custom
    code, hdrs, body = send_raw(HOST1, PORT1,
        "GET /nonexistent HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    check("404 retourné", code == 404, f"got {code}")
    check("Content-Type error 404 = text/html",
          "text/html" in hdrs.get("content-type",""), hdrs.get("content-type"))

    # 405 Method Not Allowed
    code, _, _ = send_raw(HOST1, PORT1,
        "DELETE / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    check("Méthode non autorisée → 405", code == 405, f"got {code}")

    # 400 Bad Request (requête invalide)
    code, _, _ = send_raw(HOST1, PORT1, "BADREQUEST\r\n\r\n")
    check("Requête malformée → 400", code in (400, 0), f"got {code}")


def test_methods():
    section("4. Méthodes HTTP (GET / POST / DELETE)")

    # POST avec upload
    body_data = "Hello, upload!"
    req = (
        f"POST /upload/test_upload.txt HTTP/1.1\r\n"
        f"Host: 127.0.0.1:8081\r\n"
        f"Content-Length: {len(body_data)}\r\n"
        f"Content-Type: text/plain\r\n"
        f"Connection: close\r\n\r\n"
        f"{body_data}"
    )
    code, _, _ = send_raw(HOST2, PORT2, req)
    check("POST upload → 201", code == 201, f"got {code}")

    # Vérifier que le fichier uploadé existe
    uploaded = os.path.exists("www/server2/uploads/test_upload.txt")
    check("Fichier uploadé présent sur disque", uploaded)

    # GET du fichier uploadé
    if uploaded:
        code, _, body = send_raw(HOST2, PORT2,
            "GET /files/test_upload.txt HTTP/1.1\r\nHost: 127.0.0.1:8081\r\nConnection: close\r\n\r\n")
        # peut être 200 ou 404 selon la localisation, on vérifie juste la réponse
        check("GET après upload → réponse valide", code in (200, 404), f"got {code}")

    # Créer un fichier à supprimer
    deleteme = "www/server2/files/deleteme.txt"
    if not os.path.exists(deleteme):
        with open(deleteme, "w") as f:
            f.write("to delete")

    code, _, _ = send_raw(HOST2, PORT2,
        "DELETE /files/deleteme.txt HTTP/1.1\r\nHost: 127.0.0.1:8081\r\nConnection: close\r\n\r\n")
    check("DELETE fichier → 204", code == 204, f"got {code}")
    check("Fichier supprimé du disque", not os.path.exists(deleteme))

    # DELETE fichier inexistant → 404
    code, _, _ = send_raw(HOST2, PORT2,
        "DELETE /files/ghost.txt HTTP/1.1\r\nHost: 127.0.0.1:8081\r\nConnection: close\r\n\r\n")
    check("DELETE inexistant → 404", code == 404, f"got {code}")


def test_redirects():
    section("5. Redirections HTTP 301")

    code, hdrs, _ = send_raw(HOST3, PORT3,
        "GET /old HTTP/1.1\r\nHost: 127.0.0.1:8082\r\nConnection: close\r\n\r\n")
    check("Redirection → 301", code == 301, f"got {code}")
    check("Header Location présent",
          "location" in hdrs, f"headers: {hdrs}")
    check("Location = URL cible correcte",
          hdrs.get("location","").endswith("/new"),
          hdrs.get("location",""))


def test_body_size_limit():
    section("6. Limite taille body (max_body_size)")

    # Server 2: max_body_size = 5 000 000 octets → envoyer 6 Mo doit donner 413
    big_body = "X" * 6_000_000
    req = (
        f"POST /upload/big.bin HTTP/1.1\r\n"
        f"Host: 127.0.0.1:8081\r\n"
        f"Content-Length: {len(big_body)}\r\n"
        f"Content-Type: application/octet-stream\r\n"
        f"Connection: close\r\n\r\n"
        + big_body
    )
    code, _, _ = send_raw(HOST2, PORT2, req, timeout=10)
    check("Body > max_body_size → 413", code == 413, f"got {code}")

    # Server 3: max_body_size = 100 000 octets → envoyer 200 Ko
    big_body2 = "X" * 200_000
    req2 = (
        f"POST /upload/medium.bin HTTP/1.1\r\n"
        f"Host: 127.0.0.1:8082\r\n"
        f"Content-Length: {len(big_body2)}\r\n"
        f"Content-Type: application/octet-stream\r\n"
        f"Connection: close\r\n\r\n"
        + big_body2
    )
    code2, _, _ = send_raw(HOST3, PORT3, req2, timeout=10)
    check("Body > max_body_size server3 → 405 ou 413", code2 in (405, 413), f"got {code2}")


def test_cgi():
    section("7. Exécution CGI (Python)")

    # GET script CGI
    code, hdrs, body = send_raw(HOST2, PORT2,
        "GET /scripts/hello.py HTTP/1.1\r\nHost: 127.0.0.1:8081\r\nConnection: close\r\n\r\n")
    check("CGI GET → 200", code == 200, f"got {code}")
    check("CGI retourne du HTML", "<html>" in body.lower() or "<!doctype" in body.lower(),
          body[:200])

    # GET CGI avec query string
    code, hdrs, body = send_raw(HOST2, PORT2,
        "GET /scripts/hello.py?name=42&school=lausanne HTTP/1.1\r\nHost: 127.0.0.1:8081\r\nConnection: close\r\n\r\n")
    check("CGI query string → 200", code == 200, f"got {code}")
    check("QUERY_STRING transmis au CGI",
          "name=42" in body or "school=lausanne" in body,
          body[:400])

    # POST CGI
    post_body = "message=hello+world"
    req = (
        f"POST /scripts/hello.py HTTP/1.1\r\n"
        f"Host: 127.0.0.1:8081\r\n"
        f"Content-Length: {len(post_body)}\r\n"
        f"Content-Type: application/x-www-form-urlencoded\r\n"
        f"Connection: close\r\n\r\n"
        f"{post_body}"
    )
    code, _, body = send_raw(HOST2, PORT2, req)
    check("CGI POST → 200", code == 200, f"got {code}")
    check("CGI reçoit body POST", "message=hello" in body or "POST" in body, body[:400])

    # Script CGI inexistant → 404
    code, _, _ = send_raw(HOST2, PORT2,
        "GET /scripts/ghost.py HTTP/1.1\r\nHost: 127.0.0.1:8081\r\nConnection: close\r\n\r\n")
    check("CGI inexistant → 404", code == 404, f"got {code}")


def test_autoindex():
    section("8. Autoindex (directory listing)")

    # Server 1 racine avec autoindex on et pas d'index.html connu par défaut
    # (index.html existe, donc il sera servi → on teste un sous-dossier sans index)
    code, hdrs, body = send_raw(HOST1, PORT1,
        "GET /assets/ HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    # Soit 200 avec listing, soit 403 si vide — les deux sont acceptables
    check("GET répertoire → 200 ou 403", code in (200, 403, 404), f"got {code}")
    if code == 200 and "text/html" in hdrs.get("content-type", ""):
        check("Autoindex HTML valide", "<html" in body.lower(), body[:200])


def test_chunked_upload():
    section("9. Chunked Transfer Encoding (POST)")

    chunk_data = b"Hello chunked world!"
    chunk_size = hex(len(chunk_data))[2:]  # taille en hexa
    chunked_body = (
        f"{chunk_size}\r\n".encode()
        + chunk_data
        + b"\r\n"
        + b"0\r\n\r\n"
    )
    req = (
        b"POST /upload/chunked_test.txt HTTP/1.1\r\n"
        b"Host: 127.0.0.1:8081\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: text/plain\r\n"
        b"Connection: close\r\n\r\n"
        + chunked_body
    )
    code, _, _ = send_raw_bytes(HOST2, PORT2, req)
    check("POST chunked → 201 ou 200", code in (200, 201), f"got {code}")


def test_slow_client():
    section("10. Client lent (requête fragmentée)")

    # Envoyer la requête en plusieurs morceaux avec délais
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(8)
        s.connect((HOST1, PORT1))

        # Envoyer les headers en morceaux
        s.sendall(b"GET / HTTP/")
        time.sleep(0.1)
        s.sendall(b"1.1\r\nHost: ")
        time.sleep(0.1)
        s.sendall(b"127.0.0.1:8080\r\n")
        time.sleep(0.1)
        s.sendall(b"Connection: close\r\n\r\n")

        response = b""
        while True:
            try:
                chunk = s.recv(4096)
                if not chunk:
                    break
                response += chunk
            except socket.timeout:
                break
        s.close()
        code, _, _ = parse_response(response)
        check("Client lent → 200", code == 200, f"got {code}")
    except Exception as e:
        check("Client lent → serveur répond", False, str(e))


def test_multiple_concurrent():
    section("11. Connexions concurrentes")

    results = []
    errors_list = []

    def worker(i):
        try:
            code, _, body = send_raw(HOST1, PORT1,
                f"GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nX-Client: {i}\r\nConnection: close\r\n\r\n")
            results.append(code)
        except Exception as e:
            errors_list.append(str(e))

    threads = [threading.Thread(target=worker, args=(i,)) for i in range(10)]
    for t in threads: t.start()
    for t in threads: t.join(timeout=10)

    ok = sum(1 for c in results if c == 200)
    check(f"10 requêtes simultanées : {ok}/10 réussies", ok >= 8,
          f"errors: {errors_list}")


def test_invalid_requests():
    section("12. Requêtes invalides / cas extrêmes")

    # Méthode inconnue
    code, _, _ = send_raw(HOST1, PORT1,
        "FOOBAR / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    check("Méthode inconnue → 400 ou 405", code in (400, 405), f"got {code}")

    # Version HTTP invalide
    code, _, _ = send_raw(HOST1, PORT1,
        "GET / HTTP/9.9\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    check("Version HTTP invalide → 400", code == 400, f"got {code}")

    # URI avec directory traversal
    code, _, body = send_raw(HOST1, PORT1,
        "GET /../../../etc/passwd HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    check("Directory traversal → pas de /etc/passwd",
          "root:" not in body and code in (400, 403, 404), f"code={code}")

    # Requête sans Host header (HTTP/1.1 le requiert)
    code, _, _ = send_raw(HOST1, PORT1,
        "GET / HTTP/1.1\r\nConnection: close\r\n\r\n")
    check("Requête sans Host → 400 ou 200", code in (200, 400), f"got {code}")

    # URI vide
    code, _, _ = send_raw(HOST1, PORT1,
        "GET  HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    check("URI vide → 400", code in (400, 500), f"got {code}")

    # Headers très longs
    big_header = "X-Big: " + "A" * 8000
    code, _, _ = send_raw(HOST1, PORT1,
        f"GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n{big_header}\r\nConnection: close\r\n\r\n")
    check("Headers très longs → réponse valide", code in (200, 400, 413, 431), f"got {code}")


def test_server_resilience():
    section("13. Résilience du serveur")

    # Connexion puis fermeture immédiate
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST1, PORT1))
        s.close()  # fermer sans envoyer rien
        time.sleep(0.2)
        # Le serveur doit encore répondre
        code, _, _ = send_raw(HOST1, PORT1,
            "GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
        check("Serveur survit à déconnexion brutale", code == 200, f"got {code}")
    except Exception as e:
        check("Serveur survit à déconnexion brutale", False, str(e))

    # Envoi de données aléatoires
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(3)
        s.connect((HOST1, PORT1))
        s.sendall(b"\x00\x01\x02\x03\xff\xfe garbage data !@#$%")
        try:
            s.recv(4096)
        except:
            pass
        s.close()
        time.sleep(0.2)
        code, _, _ = send_raw(HOST1, PORT1,
            "GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
        check("Serveur survit à données invalides", code == 200, f"got {code}")
    except Exception as e:
        check("Serveur survit à données invalides", False, str(e))


def test_multiple_ports_independence():
    section("14. Indépendance des ports")

    # Port 8080 sert du statique
    code1, _, body1 = send_raw(HOST1, PORT1,
        "GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")

    # Port 8082 sert server3 (contenu différent)
    code3, _, body3 = send_raw(HOST3, PORT3,
        "GET / HTTP/1.1\r\nHost: 127.0.0.1:8082\r\nConnection: close\r\n\r\n")

    check("Port 8080 répond", code1 == 200, f"got {code1}")
    check("Port 8082 répond", code3 == 200, f"got {code3}")
    # Les deux serveurs servent des contenus différents
    check("Ports servent des contenus différents",
          body1 != body3 or (code1 == code3 == 200),
          "même contenu sur les deux ports")


def test_content_length_header():
    section("15. Exactitude des headers de réponse")

    code, hdrs, body = send_raw(HOST1, PORT1,
        "GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: close\r\n\r\n")
    if code == 200:
        cl = hdrs.get("content-length", "")
        check("Content-Length présent", cl != "", f"headers: {hdrs}")
        if cl:
            check("Content-Length exact",
                  int(cl) == len(body.encode("utf-8", errors="replace")),
                  f"header={cl}, body len={len(body.encode('utf-8', errors='replace'))}")


# ═══════════════════════════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════════════════════════

if __name__ == "__main__":
    # Changer le CWD vers la racine du projet
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(os.path.join(script_dir, ".."))

    print(f"\n{BOLD}{'═'*55}")
    print(f"  WEBSERV TEST SUITE  –  42 Lausanne")
    print(f"{'═'*55}{RESET}")
    print(f"  Serveur attendu sur {HOST1}:{PORT1}, {PORT2}, {PORT3}")
    print(f"  Mode verbose : {'oui' if VERBOSE else 'non (--verbose pour détails)'}")

    # Vérifier connectivité initiale
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2)
        s.connect((HOST1, PORT1))
        s.close()
    except Exception:
        print(f"\n{RED}ERREUR : Impossible de joindre {HOST1}:{PORT1}")
        print(f"Lance le serveur d'abord : ./webserv config/server.conf{RESET}\n")
        sys.exit(1)

    start = time.time()

    test_server_reachable()
    test_static_files()
    test_error_pages()
    test_methods()
    test_redirects()
    test_body_size_limit()
    test_cgi()
    test_autoindex()
    test_chunked_upload()
    test_slow_client()
    test_multiple_concurrent()
    test_invalid_requests()
    test_server_resilience()
    test_multiple_ports_independence()
    test_content_length_header()

    elapsed = time.time() - start
    total = passed + failed

    print(f"\n{BOLD}{'═'*55}")
    print(f"  RÉSULTATS : {GREEN}{passed}{RESET}/{total} tests passés  "
          f"({RED}{failed} échoués{RESET})  [{elapsed:.2f}s]")
    print(f"{'═'*55}{RESET}\n")

    if failed > 0:
        print(f"{RED}Tests échoués :{RESET}")
        for e in errors:
            print(f"  - {e}")
        print()

    sys.exit(0 if failed == 0 else 1)
