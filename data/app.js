import { createJoystick } from "./joystick.js";
import {
  bindHoldButtons,
  initUI,
  setCameraStatus,
  setConnectionStatus,
  setMode,
  updateCustomSlider,
  showConfirmPopup,
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
  fwdPower: 180,
  bwdPower: 180,
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
      speed: (dir === "BACKWARD") ? state.bwdPower : state.fwdPower,
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
  if (enabled) {
    elements.cameraStream.src = `http://${location.hostname}:81/stream?t=${Date.now()}`;
  } else {
    elements.cameraStream.src = "";
  }
}

wsClient.onStatus = (online) => {
  setConnectionStatus(elements, online);
};

// Initialize component visuals
wsClient.connect();
setMode(elements, state.mode);
setConnectionStatus(elements, false);
setCameraStatus(elements, false);
updateCustomSlider(elements.brightnessSlider, state.flashLevel);
updateCustomSlider(elements.driveSensitivity, ((state.drive - 0.4)/(1.5 - 0.4))*100);
updateCustomSlider(elements.turnSensitivity, ((state.turn - 0.4)/(1.5 - 0.4))*100);

createJoystick(elements.joystickZone, elements.joystickKnob, {
  onMove: ({ x, y }) => {
    state.joystick.active = true;
    state.joystick.x = Math.round(x * (state.fwdPower / 255));
    state.joystick.y = Math.round(y * (state.fwdPower / 255));
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

// Slider handlers
elements.brightnessSlider.addEventListener("input", (e) => {
  const percent = Number(e.target.value);
  state.flashLevel = Math.round((percent / 100) * 255);
  updateCustomSlider(elements.brightnessSlider, percent);
  sendFlash();
});

elements.driveSensitivity.addEventListener("input", (e) => {
  const val = Number(e.target.value);
  state.drive = val;
  const min = 0.4, max = 1.5;
  updateCustomSlider(elements.driveSensitivity, ((val - min) / (max - min)) * 100);
  sendSettings();
});

elements.turnSensitivity.addEventListener("input", (e) => {
  const val = Number(e.target.value);
  state.turn = val;
  const min = 0.4, max = 1.5;
  updateCustomSlider(elements.turnSensitivity, ((val - min) / (max - min)) * 100);
  sendSettings();
});

// Segmented toggles with confirm popup
elements.fwdPowerGroup.forEach(btn => {
  btn.addEventListener("click", () => {
    if(btn.classList.contains("active")) return;
    showConfirmPopup(elements, () => {
      elements.fwdPowerGroup.forEach(b => b.classList.remove("active"));
      btn.classList.add("active");
      state.fwdPower = parseInt(btn.dataset.val);
    });
  });
});

elements.bwdPowerGroup.forEach(btn => {
  btn.addEventListener("click", () => {
    if(btn.classList.contains("active")) return;
    showConfirmPopup(elements, () => {
      elements.bwdPowerGroup.forEach(b => b.classList.remove("active"));
      btn.classList.add("active");
      state.bwdPower = parseInt(btn.dataset.val);
    });
  });
});
