import os
os.environ["User-Agent"] = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36"
os.environ["path"] = "/cgi/example_cgi.py?hejsandsa=sasda"
os.environ["Host"] = "192.168.1.111:8080"
os.environ["Upgrade-Insecure-Requests"] = "1"
os.environ["Cache-Control"] = "max-age=0"
os.environ["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8"
os.environ["Accept-Encoding"] = "gzip, deflate, sdch"
os.environ["Connection"] = "keep-alive"
os.environ["Accept-Language"] = "sv-SE,sv;q=0.8,en-US;q=0.6,en;q=0.4"
import example_cgi