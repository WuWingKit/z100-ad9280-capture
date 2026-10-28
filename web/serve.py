# 本文件的执行方式、输入输出及与其它模块的关系见 docs/CODE_GUIDE.md。
"""Local-only static dashboard server. Does not open a serial port."""

from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
from functools import partial
from pathlib import Path
import argparse, webbrowser


def main():
    # 1. 本服务只提供web目录静态文件；不包含串口或硬件下载接口。
    p = argparse.ArgumentParser()
    p.add_argument("--port", type=int, default=8765)
    p.add_argument("--open", action="store_true")
    a = p.parse_args()
    root = Path(__file__).resolve().parent

    # 2. 禁用缓存：修改HTML/JS后刷新即可看到新代码。
    class Handler(SimpleHTTPRequestHandler):
        def end_headers(self):
            self.send_header("Cache-Control", "no-store")
            super().end_headers()

    server = ThreadingHTTPServer(
        ("127.0.0.1", a.port), partial(Handler, directory=str(root))
    )
    # 3. 仅绑定回环地址，不把工程文件暴露给局域网其它设备。
    url = f"http://127.0.0.1:{a.port}"
    print("Z100 dashboard: " + url + " (Ctrl+C to stop)", flush=True)
    if a.open:
        webbrowser.open(url)
    # 4. --open只打开浏览器；串口仍需用户点击网页按钮选择并连接。
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
