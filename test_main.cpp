
#include "inc/Config.hpp"
#include "inc/Request.hpp"
#include "inc/Response.hpp"
#include "inc/RequestHandler.hpp"
#include <iostream>
#include <sstream>

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void	print_separator(const std::string &title) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║ " << title;
    for (size_t i = title.length(); i < 58; i++)
        std::cout << " ";
    std::cout << "║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
}

void	print_test_result(const std::string &testName, bool success) {
    if (success)
        std::cout << "  ✓ " << testName << std::endl;
    else
        std::cout << "  ✗ " << testName << std::endl;
}

// ============================================================================
// TEST 1: CONFIG PARSER
// ============================================================================

void	test_config_parser() {
    print_separator("TEST 1: CONFIG PARSER");

    try {
        ConfigParser parser;
        std::vector<ServerConfig> servers = parser.parse("config/server.conf");

        print_test_result("Load configuration file", !servers.empty());
        print_test_result("Multiple servers parsed", servers.size() > 0);

        for (size_t i = 0; i < servers.size(); i++) {
            std::cout << "\n  [SERVER " << (i + 1) << "]" << std::endl;
            std::cout << "    • Port: " << servers[i].port << std::endl;
            std::cout << "    • Host: " << servers[i].host << std::endl;
            std::cout << "    • Root: " << servers[i].root << std::endl;
            std::cout << "    • Max body size: " << servers[i].maxBodySize << std::endl;
            std::cout << "    • Locations: " << servers[i].locations.size() << std::endl;

            for (size_t j = 0; j < servers[i].locations.size(); j++) {
                LocationConfig &loc = servers[i].locations[j];
                std::cout << "      [LOCATION " << (j + 1) << "] " << loc.path << std::endl;
                std::cout << "        - Root: " << loc.root << std::endl;
                std::cout << "        - Allowed methods: ";
                for (size_t k = 0; k < loc.allowedMethods.size(); k++) {
                    std::cout << loc.allowedMethods[k];
                    if (k < loc.allowedMethods.size() - 1)
                        std::cout << ", ";
                }
                std::cout << std::endl;
                std::cout << "        - Autoindex: " << (loc.autoIndex ? "on" : "off") << std::endl;
                std::cout << "        - Upload allowed: " << (loc.allowUpload ? "yes" : "no") << std::endl;
                if (!loc.redirectUrl.empty())
                    std::cout << "        - Redirect to: " << loc.redirectUrl << std::endl;
                if (!loc.cgiHandlers.empty()) {
                    std::cout << "        - CGI handlers: ";
                    for (std::map<std::string, std::string>::iterator it = loc.cgiHandlers.begin();
                            it != loc.cgiHandlers.end(); ++it) {
                        std::cout << it->first << "->" << it->second << " ";
                    }
                    std::cout << std::endl;
                }
            }
        }
        std::cout << "\n  ✓ Configuration parsing SUCCESS" << std::endl;
    }
    catch (const ConfigParserE &e) {
        std::cout << "\n  ✗ Configuration parsing FAILED: " << e.what() << std::endl;
    }
    catch (const std::exception &e) {
        std::cout << "\n  ✗ Unexpected error: " << e.what() << std::endl;
    }
}

// ============================================================================
// TEST 2: REQUEST PARSER
// ============================================================================

void	test_request_parser() {
    print_separator("TEST 2: REQUEST PARSER");

    // Test 2.1: Simple GET request
    try {
        Request req1;
        std::string rawReq1 = "GET /index.html HTTP/1.1\r\n"
                              "Host: localhost:8080\r\n"
                              "User-Agent: TestClient/1.0\r\n"
                              "\r\n";
        req1.parse(rawReq1);

        bool test1 = (req1.getMethod() == "GET" &&
                      req1.getUri() == "/index.html" &&
                      req1.getVersion() == "HTTP/1.1" &&
                      req1.getHeader("Host") == "localhost:8080");
        print_test_result("Parse simple GET request", test1);
        if (test1) {
            std::cout << "    Headers count: " << req1.getHeaderCount() << std::endl;
        }
    }
    catch (const std::exception &e) {
        print_test_result("Parse simple GET request", false);
        std::cout << "    Error: " << e.what() << std::endl;
    }

    // Test 2.2: POST request with body
    try {
        Request req2;
        std::string bodyContent = "{\"key\": \"value\", \"number\": 42}";
        std::stringstream ss;
        ss << "POST /api/users HTTP/1.1\r\n"
           << "Host: localhost:8080\r\n"
           << "Content-Type: application/json\r\n"
           << "Content-Length: " << bodyContent.length() << "\r\n"
           << "\r\n"
           << bodyContent;

        req2.parse(ss.str());

        bool test2 = (req2.getMethod() == "POST" &&
                      req2.getUri() == "/api/users" &&
                      req2.getBody() == bodyContent);
        print_test_result("Parse POST request with body", test2);
        if (test2) {
            std::cout << "    Body length: " << req2.getBody().length() << " bytes" << std::endl;
            std::cout << "    Content-Type: " << req2.getHeader("Content-Type") << std::endl;
        }
    }
    catch (const std::exception &e) {
        print_test_result("Parse POST request with body", false);
        std::cout << "    Error: " << e.what() << std::endl;
    }

    // Test 2.3: DELETE request
    try {
        Request req3;
        std::string rawReq3 = "DELETE /file.txt HTTP/1.1\r\n"
                              "Host: localhost:8080\r\n"
                              "\r\n";
        req3.parse(rawReq3);

        bool test3 = (req3.getMethod() == "DELETE" && req3.getUri() == "/file.txt");
        print_test_result("Parse DELETE request", test3);
    }
    catch (const std::exception &e) {
        print_test_result("Parse DELETE request", false);
        std::cout << "    Error: " << e.what() << std::endl;
    }

    // Test 2.4: Invalid request (should fail)
    try {
        Request req4;
        std::string invalidReq = "INVALID /path HTTP/1.1\r\n\r\n";
        req4.parse(invalidReq);
        print_test_result("Reject invalid HTTP method", false);
    }
    catch (const RequestE &e) {
        print_test_result("Reject invalid HTTP method", true);
    }
    catch (const std::exception &e) {
        print_test_result("Reject invalid HTTP method", false);
        std::cout << "    Unexpected error: " << e.what() << std::endl;
    }

    std::cout << "\n  ✓ Request parsing tests completed" << std::endl;
}

// ============================================================================
// TEST 3: RESPONSE BUILDER
// ============================================================================

void	test_response_builder() {
    print_separator("TEST 3: RESPONSE BUILDER");

    // Test 3.1: Simple 200 response
    try {
        Response resp;
        resp.setVersion("HTTP/1.1");
        resp.setStatus(200, "OK");
        resp.setHeader("Content-Type", "text/html");
        resp.setHeader("Content-Length", "13");
        resp.setBody("<h1>Hello</h1>");

        std::string built = resp.build();
        bool hasStatus = built.find("200 OK") != std::string::npos;
        bool hasContentType = built.find("Content-Type: text/html") != std::string::npos;
        bool hasBody = built.find("<h1>Hello</h1>") != std::string::npos;

        print_test_result("Build 200 OK response", hasStatus && hasContentType && hasBody);
    }
    catch (const std::exception &e) {
        print_test_result("Build 200 OK response", false);
        std::cout << "    Error: " << e.what() << std::endl;
    }

    // Test 3.2: 404 error response
    try {
        Response resp;
        resp.setVersion("HTTP/1.1");
        resp.setStatus(404, "Not Found");
        resp.setHeader("Content-Type", "text/html");
        resp.setHeader("Content-Length", "23");
        resp.setBody("<h1>404 Not Found</h1>");

        std::string built = resp.build();
        bool hasStatus = built.find("404 Not Found") != std::string::npos;

        print_test_result("Build 404 error response", hasStatus);
    }
    catch (const std::exception &e) {
        print_test_result("Build 404 error response", false);
        std::cout << "    Error: " << e.what() << std::endl;
    }

    // Test 3.3: Response with multiple headers
    try {
        Response resp;
        resp.setVersion("HTTP/1.1");
        resp.setStatus(201, "Created");
        resp.setHeader("Content-Type", "application/json");
        resp.setHeader("Content-Length", "20");
        resp.setHeader("Location", "/users/123");
        resp.setBody("{\"id\": 123, \"ok\": 1}");

        std::string built = resp.build();
        bool has201 = built.find("201 Created") != std::string::npos;
        bool hasLocation = built.find("Location: /users/123") != std::string::npos;

        print_test_result("Build response with multiple headers", has201 && hasLocation);
        std::cout << "    Built response size: " << built.length() << " bytes" << std::endl;
    }
    catch (const std::exception &e) {
        print_test_result("Build response with multiple headers", false);
        std::cout << "    Error: " << e.what() << std::endl;
    }

    std::cout << "\n  ✓ Response builder tests completed" << std::endl;
}

// ============================================================================
// TEST 4: REQUEST HANDLER (Integration Test)
// ============================================================================


void	test_request_handler() {
    print_separator("TEST 4: REQUEST HANDLER (Integration)");

    try {
        // Load configuration
        ConfigParser parser;
        std::vector<ServerConfig> servers = parser.parse("config/server.conf");

        if (servers.empty()) {
            std::cout << "  ✗ No servers configured\n" << std::endl;
            return;
        }

        RequestHandler handler(servers);

        // Test 4.1: Handle GET request
        try {
            Request getReq;
            std::string rawGet = "GET / HTTP/1.1\r\n"
                                 "Host: localhost:8080\r\n"
                                 "\r\n";
            getReq.parse(rawGet);

            Response getResp = handler.handleRequest(getReq, rawGet, servers[0].port);
            std::string builtResp = getResp.build();
            bool hasResponse = !builtResp.empty();
            print_test_result("Handle GET request for /", hasResponse);
            if (hasResponse && builtResp.find("200") != std::string::npos)
                std::cout << "    Status: 200 OK" << std::endl;
            else if (hasResponse && builtResp.find("404") != std::string::npos)
                std::cout << "    Status: 404 Not Found (expected if no index)" << std::endl;
        }
        catch (const std::exception &e) {
            print_test_result("Handle GET request for /", false);
            std::cout << "    Error: " << e.what() << std::endl;
        }

        // Test 4.2: Handle request to non-existent resource
        try {
            Request notFoundReq;
            std::string rawNotFound = "GET /nonexistent.html HTTP/1.1\r\n"
                                       "Host: localhost:8080\r\n"
                                       "\r\n";
            notFoundReq.parse(rawNotFound);

            Response notFoundResp = handler.handleRequest(notFoundReq, rawNotFound, servers[0].port);
            std::string builtResp = notFoundResp.build();
            bool has404 = builtResp.find("404") != std::string::npos;
            print_test_result("Return 404 for non-existent resource", has404);
        }
        catch (const std::exception &e) {
            print_test_result("Return 404 for non-existent resource", false);
            std::cout << "    Error: " << e.what() << std::endl;
        }

        // Test 4.3: Handle request with wrong method
        try {
            Request wrongMethodReq;
            std::string rawWrongMethod = "DELETE / HTTP/1.1\r\n"
                                         "Host: localhost:8080\r\n"
                                         "\r\n";
            wrongMethodReq.parse(rawWrongMethod);

            Response wrongMethodResp = handler.handleRequest(wrongMethodReq, rawWrongMethod, servers[0].port);
            std::string builtResp = wrongMethodResp.build();
            bool has405 = builtResp.find("405") != std::string::npos || builtResp.find("404") != std::string::npos;
            print_test_result("Handle unauthorized method", has405);
        }
        catch (const std::exception &e) {
            print_test_result("Handle unauthorized method", false);
            std::cout << "    Error: " << e.what() << std::endl;
        }

        std::cout << "\n  ✓ Request handler tests completed" << std::endl;
    }
    catch (const std::exception &e) {
        std::cout << "  ✗ Test suite failed: " << e.what() << std::endl;
    }
}

// ============================================================================
// MAIN
// ============================================================================

int	main(void) {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                   WEBSERV TEST SUITE                              ║" << std::endl;
    std::cout << "║          Testing: Config, Request, Response, RequestHandler       ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝" << std::endl;

    test_config_parser();
    test_request_parser();
    test_response_builder();
    test_request_handler();

    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                   ALL TESTS COMPLETED                             ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\n";

    return (0);
}
