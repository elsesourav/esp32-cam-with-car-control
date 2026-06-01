import { createJoystick } from "./joystick.js";
import {
  bindHoldButtons,
  initUI,
  setConnectionStatus,
  setMode,
  updateCustomSlider,
  showConfirmPopup,
} from "./ui.js";
import { WsClient, buildMessage } from "./websocket.js";
import { updateTelemetry } from "./telemetry.js";
import { initCamera } from "./camera.js";

const elements = initUI();
const wsUrl = `ws://${location.host}/ws`;
const wsClient = new WsClient(wsUrl);

const STORAGE_KEY = "rc_state_v1";

const defaultState = {
  mode: "joystick",
  joystick: { x: 0, y: 0, active: false },
  flashLevel: 0,
  cameraLive: false,
  drive: 1,
  turn: 1,
  fwdPower: 180,
  bwdPower: 180,
  moveLock: null, // "FORWARD" | "BACKWARD" | null
};

// Load saved settings
const saved = JSON.parse(localStorage.getItem(STORAGE_KEY) || "{}");
const state = { ...defaultState, ...saved };

// Ensure transient states are always reset on reload
state.joystick = { x: 0, y: 0, active: false };
state.cameraLive = false;
state.moveLock = null;

function saveState() {
  localStorage.setItem(
    STORAGE_KEY,
    JSON.stringify({
      mode: state.mode,
      flashLevel: state.flashLevel,
      drive: state.drive,
      turn: state.turn,
      fwdPower: state.fwdPower,
      bwdPower: state.bwdPower,
    }),
  );
}

function updateBadges() {
  if (elements.valDrive)
    elements.valDrive.textContent = Number(state.drive).toFixed(2);
  if (elements.valTurn)
    elements.valTurn.textContent = Number(state.turn).toFixed(2);
  if (elements.valLight)
    elements.valLight.textContent =
      Math.round((state.flashLevel / 255) * 100) + "%";
}

function initSegments() {
  elements.fwdPowerGroup.forEach((b) => {
    b.classList.toggle("active", parseInt(b.dataset.val) === state.fwdPower);
  });
  elements.bwdPowerGroup.forEach((b) => {
    b.classList.toggle("active", parseInt(b.dataset.val) === state.bwdPower);
  });
}

function updateButtonUI() {
  elements.buttonZone.querySelectorAll("[data-dir]").forEach((btn) => {
    btn.classList.remove("active-lock");
    if (state.moveLock && btn.dataset.dir === state.moveLock) {
      btn.classList.add("active-lock");
    }
  });
}

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
      speed: dir === "BACKWARD" ? state.bwdPower : state.fwdPower,
      turn: 120, // default turning power bias
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

// --- Initialization ---

wsClient.connect();
setMode(elements, state.mode);
setConnectionStatus(elements, false);

// Initialize camera module (handles start/stop/stream)
const camera = initCamera(elements, wsClient, buildMessage);

// Set initial slider DOM values
elements.brightnessSlider.value = Math.round((state.flashLevel / 255) * 100);
elements.driveSensitivity.value = state.drive;
elements.turnSensitivity.value = state.turn;

updateCustomSlider(elements.brightnessSlider, elements.brightnessSlider.value);
updateCustomSlider(
  elements.driveSensitivity,
  ((state.drive - 0.4) / (1.5 - 0.4)) * 100,
);
updateCustomSlider(
  elements.turnSensitivity,
  ((state.turn - 0.4) / (1.5 - 0.4)) * 100,
);

updateBadges();
initSegments();
updateButtonUI();

// --- WebSocket Callbacks ---

wsClient.onStatus = (online) => {
  setConnectionStatus(elements, online);
};

// Handle incoming telemetry messages from ESP32
wsClient.onMessage = (rawData) => {
  const params = new URLSearchParams(rawData);
  const type = params.get("type");
  if (type === "telemetry") {
    updateTelemetry(elements, {
      x: params.get("x"),
      y: params.get("y"),
      z: params.get("z"),
      a: params.get("a"),
      tilt: params.get("tilt"),
      state: params.get("state"),
      conn: params.get("conn"),
    });
  }
};

// --- Input Controls ---

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

// Reduced from 60ms to 50ms for snappier joystick response.
setInterval(() => {
  if (state.mode === "joystick" && state.joystick.active) {
    sendMoveJoystick();
  }
}, 30);

elements.modeJoystick.addEventListener("click", () => {
  state.mode = "joystick";
  saveState();
  setMode(elements, state.mode);
});

elements.modeButtons.addEventListener("click", () => {
  state.mode = "button";
  saveState();
  setMode(elements, state.mode);
});

// Setting Up Lock Controls vs Hold Controls
const lockControls = Array.from(
  elements.buttonZone.querySelectorAll(
    "[data-dir='FORWARD'], [data-dir='BACKWARD'], [data-dir='STOP']",
  ),
);
const holdControls = Array.from(
  elements.buttonZone.querySelectorAll("[data-dir='LEFT'], [data-dir='RIGHT']"),
);

let lockIntervalId = null;

function handleLockPress(dir) {
  if (state.mode !== "button") return;
  if (lockIntervalId) {
    clearInterval(lockIntervalId);
    lockIntervalId = null;
  }

  if (dir === "STOP") {
    state.moveLock = null;
    sendMoveDir("STOP");
  } else {
    // If switching direction
    state.moveLock = dir;
    sendMoveDir(dir);
    lockIntervalId = setInterval(() => sendMoveDir(state.moveLock), 120);
  }
  updateButtonUI();
}

lockControls.forEach((btn) => {
  btn.addEventListener("pointerdown", (event) => {
    event.preventDefault();
    handleLockPress(btn.dataset.dir);
  });
});

// Drive-mixing helper: blend a locked direction with a turn direction
// into a compound direction (e.g. FORWARD + LEFT → FORWARD_LEFT).
function blendDirection(lockDir, turnDir) {
  if (!lockDir) return turnDir;
  return lockDir + "_" + turnDir; // e.g. "FORWARD_LEFT", "BACKWARD_RIGHT"
}

bindHoldButtons(
  holdControls,
  (dir) => {
    if (state.mode === "button") {
      // Pause lock interval while turning
      if (lockIntervalId) {
        clearInterval(lockIntervalId);
        lockIntervalId = null;
      }
      // Blend: if FORWARD/BACKWARD is locked, combine with LEFT/RIGHT
      // into a smooth arc instead of cancelling forward movement.
      const blended = blendDirection(state.moveLock, dir);
      sendMoveDir(blended);
      // Continue sending the blended command while held
      lockIntervalId = setInterval(() => sendMoveDir(blended), 120);
    }
  },
  () => {
    if (state.mode === "button") {
      if (lockIntervalId) {
        clearInterval(lockIntervalId);
        lockIntervalId = null;
      }
      if (state.moveLock) {
        // Resume pure locked direction
        sendMoveDir(state.moveLock);
        lockIntervalId = setInterval(() => sendMoveDir(state.moveLock), 120);
      } else {
        sendMoveDir("STOP");
      }
    }
  },
);

// Slider handlers
elements.brightnessSlider.addEventListener("input", (e) => {
  const percent = Number(e.target.value);
  state.flashLevel = Math.round((percent / 100) * 255);
  updateCustomSlider(elements.brightnessSlider, percent);
  updateBadges();
  sendFlash();
  saveState();
});

elements.driveSensitivity.addEventListener("input", (e) => {
  const val = Number(e.target.value);
  state.drive = val;
  const min = 0.4,
    max = 1.5;
  updateCustomSlider(
    elements.driveSensitivity,
    ((val - min) / (max - min)) * 100,
  );
  updateBadges();
  sendSettings();
  saveState();
});

elements.turnSensitivity.addEventListener("input", (e) => {
  const val = Number(e.target.value);
  state.turn = val;
  const min = 0.4,
    max = 1.5;
  updateCustomSlider(
    elements.turnSensitivity,
    ((val - min) / (max - min)) * 100,
  );
  updateBadges();
  sendSettings();
  saveState();
});

// Segmented toggles with confirm popup
elements.fwdPowerGroup.forEach((btn) => {
  btn.addEventListener("click", () => {
    if (btn.classList.contains("active")) return;
    showConfirmPopup(elements, () => {
      elements.fwdPowerGroup.forEach((b) => b.classList.remove("active"));
      btn.classList.add("active");
      state.fwdPower = parseInt(btn.dataset.val);
      saveState();
    });
  });
});

elements.bwdPowerGroup.forEach((btn) => {
  btn.addEventListener("click", () => {
    if (btn.classList.contains("active")) return;
    showConfirmPopup(elements, () => {
      elements.bwdPowerGroup.forEach((b) => b.classList.remove("active"));
      btn.classList.add("active");
      state.bwdPower = parseInt(btn.dataset.val);
      saveState();
    });
  });
});
