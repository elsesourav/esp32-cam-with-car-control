const fs = require('fs');
const path = require('path');

const files = [
    "index.html",
    "style.css",
    "app.js",
    "joystick.js",
    "websocket.js",
    "ui.js",
    "telemetry.js",
    "camera.js"
];

const out = ["#pragma once", "", "#include <Arduino.h>", ""];

for (const f of files) {
    const filePath = path.join(__dirname, "..", "data", f);
    const content = fs.readFileSync(filePath, "utf-8");
    
    const varName = f.replace(/\./g, "_");
    
    out.push(`const char embedded_${varName}[] PROGMEM = ${JSON.stringify(content)};`);
    out.push(`const size_t embedded_${varName}_len = sizeof(embedded_${varName}) - 1;`);
    out.push("");
}

fs.writeFileSync(path.join(__dirname, "embedded_files.h"), out.join("\n"));
console.log("embedded_files.h generated successfully.");
