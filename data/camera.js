// camera.js — Camera start/stop/stream module.
// Extracted from app.js for modularity.

export function initCamera(elements, wsClient, buildMessage) {
  function setCameraStream(enabled) {
    elements.cameraStatus.textContent = enabled ? "LIVE" : "OFFLINE";
    elements.cameraStatus.style.color = enabled ? "#3dd6f5" : "#ff5d5d";

    if (enabled) {
      elements.cameraStream.style.display = "block";
      elements.cameraPlaceholder.style.display = "none";
      elements.cameraStream.src = `http://${location.hostname}:81/stream?t=${Date.now()}`;
    } else {
      elements.cameraStream.style.display = "none";
      elements.cameraPlaceholder.style.display = "flex";
      elements.cameraStream.src = "";
    }
  }

  elements.cameraStart.addEventListener("click", () => {
    // 1. Tell ESP32 to power on the camera hardware
    wsClient.send(buildMessage("camera", { action: "start" }));
    
    // 2. Wait 800ms for hardware to boot up, THEN fetch the stream
    setTimeout(() => {
      setCameraStream(true);
    }, 800);
  });

  elements.cameraStop.addEventListener("click", () => {
    setCameraStream(false);
    wsClient.send(buildMessage("camera", { action: "stop" }));
  });

  // Initialize as offline.
  setCameraStream(false);

  return { setCameraStream };
}
