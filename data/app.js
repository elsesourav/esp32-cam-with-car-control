import { createJoystick } from "./joystick.js";
import {
  bindHoldButtons,
  initUI,
  setCameraStatus,
  setConnectionStatus,
  setMode,
  updateFlashUI,
} from "./ui.js";
import { WsClient, buildMessage } from "./websocket.js";

const elements = initUI();
const wsUrl = `ws://${location.host}/ws`;
const wsClient = new WsClient(wsUrl);

const state = {
  mode: "joystick",
  joystick: { x: 0, y: 0, active: false },
  
  flashLevel: 0,
  cameraLive: false,
  drive: 1,
  turn: 1,
};

function sendMoveJoystick() {
  wsClient.send(
    buildMessage("move", {
      mode: "joystick",
      x: state.joystick.x,
      y: state.joystick.y,
    }),
  );
}

function sendMoveDir(dir) {
  wsClient.send(
    buildMessage("move", {
      mode: "button",
      dir,
      speed: 180,
      turn: 120,
    }),
  );
}

function sendSettings() {
  wsClient.send(
    buildMessage("settings", {
      drive: state.drive,
      turn: state.turn,
    }),
  );
}

function sendFlash() {
  wsClient.send(
    buildMessage("flash", {
      on: state.flashLevel > 0 ? 1 : 0,
      level: state.flashLevel,
    }),
  );
}

function sendCamera(action) {
  wsClient.send(buildMessage("camera", { action }));
}

function setCameraStream(enabled) {
  state.cameraLive = enabled;
  setCameraStatus(elements, enabled);
  const streamElement = document.getElementById("camera-stream");

  if (enabled) {
    streamElement.src = `http://${location.hostname}:81/stream?t=${Date.now()}`;
  } else {
    streamElement.src = "";
  }
}

wsClient.onStatus = (online) => {
  setConnectionStatus(elements, online);
};

wsClient.connect();

setMode(elements, state.mode);
setConnectionStatus(elements, false);
setCameraStatus(elements, false);
updateFlashUI(elements, state.flashLevel);

createJoystick(elements.joystickZone, elements.joystickKnob, {
  onMove: ({ x, y }) => {
    state.joystick.active = true;
    state.joystick.x = x;
    state.joystick.y = y;
  },
  onEnd: () => {
    state.joystick.active = false;
    state.joystick.x = 0;
    state.joystick.y = 0;
    sendMoveDir("STOP");
  },
});

setInterval(() => {
  if (state.mode === "joystick" && state.joystick.active) {
    sendMoveJoystick();
  }
}, 60);

elements.modeJoystick.addEventListener("click", () => {
  state.mode = "joystick";
  setMode(elements, state.mode);
});

elements.modeButtons.addEventListener("click", () => {
  state.mode = "button";
  setMode(elements, state.mode);
});

const buttonControls = Array.from(
  elements.buttonZone.querySelectorAll("[data-dir]"),
);

bindHoldButtons(
  buttonControls,
  (dir) => {
    if (state.mode === "button") {
      sendMoveDir(dir);
    }
  },
  () => {
    sendMoveDir("STOP");
  },
);

elements.cameraStart.addEventListener("click", () => {
  setCameraStream(true);
  sendCamera("start");
});

elements.cameraStop.addEventListener("click", () => {
  setCameraStream(false);
  sendCamera("stop");
});

elements.brightnessSlider.addEventListener("input", (e) => {
  const percent = Number(e.target.value);
  state.flashLevel = Math.round((percent / 100) * 255);
  updateFlashUI(elements, state.flashLevel);
  sendFlash();
});

// Initialize brightness visual state
elements.brightnessSlider.style.setProperty(
  "--slider-value",
  elements.brightnessSlider.value + "%",
);





elements.driveSensitivity.addEventListener("input", (event) => {
  state.drive = Number(event.target.value);
  sendSettings();
});

elements.turnSensitivity.addEventListener("input", (event) => {
  state.turn = Number(event.target.value);
  sendSettings();
});
