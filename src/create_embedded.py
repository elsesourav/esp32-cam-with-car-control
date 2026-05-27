import os
import json

files = [
    "index.html",
    "style.css",
    "app.js",
    "joystick.js",
    "websocket.js",
    "ui.js"
]

out = ["#pragma once", "", "#include <Arduino.h>", ""]

for f in files:
    path = os.path.join(os.path.dirname(__file__), "..", "data", f)
    with open(path, "r", encoding="utf-8") as file:
        content = file.read()
        
    var_name = f.replace(".", "_")
    
    out.append(f"const char embedded_{var_name}[] PROGMEM = {json.dumps(content)};")
    out.append(f"const size_t embedded_{var_name}_len = sizeof(embedded_{var_name}) - 1;")
    out.append("")

with open(os.path.join(os.path.dirname(__file__), "embedded_files.h"), "w") as file:
    file.write("\n".join(out))

