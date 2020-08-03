from bs4 import BeautifulSoup
from pathlib import Path
import urllib
import requests

session = requests.Session()
adapter = requests.adapters.HTTPAdapter(pool_connections=100, pool_maxsize=2048)
session.mount("", adapter)

for path in Path("..").glob("**/*.html"):
    with open(path, "r") as fi:
        raw_html = fi.read()
    soup = BeautifulSoup(raw_html, "lxml")
    anchors = soup.find_all("a")
    for anchor in anchors:
        link = anchor.get("href")
        parsed = urllib.parse.urlparse(link)
        if len(parsed.scheme) != 0:
            url = urllib.parse.urlunparse(parsed)
            req = session.head(url)

            log = f"{path} {url} {req.status_code}"

            status_class = req.status_code // 100
            if status_class == 4 or status_class == 5:
                log = "Error   " + log
            else:
                log = "Success " + log

            print(log)
