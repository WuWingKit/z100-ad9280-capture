"""Local-only static dashboard server. Does not open a serial port."""
from http.server import ThreadingHTTPServer,SimpleHTTPRequestHandler
from functools import partial
from pathlib import Path
import argparse,webbrowser

def main():
    p=argparse.ArgumentParser();p.add_argument('--port',type=int,default=8765);p.add_argument('--open',action='store_true');a=p.parse_args()
    root=Path(__file__).resolve().parent
    class Handler(SimpleHTTPRequestHandler):
        def end_headers(self):
            self.send_header('Cache-Control','no-store');super().end_headers()
    server=ThreadingHTTPServer(('127.0.0.1',a.port),partial(Handler,directory=str(root)))
    url=f'http://127.0.0.1:{a.port}'
    print('Z100 dashboard: '+url+' (Ctrl+C to stop)',flush=True)
    if a.open:webbrowser.open(url)
    try:server.serve_forever()
    except KeyboardInterrupt:pass
    finally:server.server_close()
if __name__=='__main__':main()
