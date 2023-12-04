from bs4 import BeautifulSoup
from pathlib import Path
import urllib
import requests

session = requests.Session()
adapter = requests.adapters.HTTPAdapter(pool_connections=100, pool_maxsize=2048)
session.mount("", adapter)

log_text = ""
visited_url = set()
for path in Path("..").glob("**/*.html"):
    if "rewrite" in path.parts:
        continue

    with open(path, "r") as fi:
        raw_html = fi.read()
    soup = BeautifulSoup(raw_html, "lxml")
    anchors = soup.find_all("a")
    for anchor in anchors:
        link = anchor.get("href")
        parsed = urllib.parse.urlparse(link)
        if len(parsed.scheme) == 0:
            continue

        url = urllib.parse.urlunparse(parsed)
        if url in visited_url:
            continue
        visited_url.add(url)

        try:
            req = session.head(url)
        except requests.exceptions.ConnectionError:
            text = f"No Connection " + url
            print(text)
            log_text += text + "\n"
            continue

        status_text = f"{path} {url} {req.status_code}"

        status_class = req.status_code // 100
        if status_class == 4 or status_class == 5:
            status_text = "Error   " + status_text
        else:
            status_text = "Success " + status_text

        print(status_text)
        log_text += status_text + "\n"

with open("verify.txt", "w", encoding="utf-8") as fi:
    fi.write(log_text)
