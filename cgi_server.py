import http.server
import os

class CustomCGIHTTPRequestHandler(http.server.CGIHTTPRequestHandler):
    def do_GET(self):
        # If the request is for "/", redirect to the CGI program
        if self.path == "/":
            self.send_response(301)
            self.send_header("Location", "/cgi-bin/cgi_id.exe")
            self.end_headers()
        else:
            super().do_GET()

# Set the web root and start server
os.chdir("www")  # Ensure we serve from the correct directory
server_address = ("", 8000)  # Bind to port 8000
httpd = http.server.HTTPServer(server_address, CustomCGIHTTPRequestHandler)

print("Serving on port 8000...")
httpd.serve_forever()

