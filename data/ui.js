export function initUI() {
  const elements = {
    wsDot: document.getElementById("ws-dot"),
    wsStatus: document.getElementById("ws-status"),
    cameraStatus: document.getElementById("camera-status"),
    cameraStream: document.getElementById("camera-stream"),
    cameraStart: document.getElementById("camera-start"),
    cameraStop: document.getElementById("camera-stop"),
    brightnessSlider: document.getElementById("brightness-slider"),
    modeJoystick: document.getElementById("mode-joystick"),
    modeButtons: document.getElementById("mode-buttons"),
    joystickZone: document.getElementById("joystick-zone"),
    joystickKnob: document.getElementById("joystick-knob"),
    buttonZone: document.getElementById("button-zone"),
    driveSensitivity: document.getElementById("drive-sensitivity"),
    turnSensitivity: document.getElementById("turn-sensitivity"),
  };

  return elements;
}

export function setConnectionStatus(elements, online) {
  elements.wsDot.style.background = online ? "#3dd6f5" : "#ff5d5d";
  elements.wsDot.style.boxShadow = online
    ? "0 0 12px rgba(61, 214, 245, 0.6)"
    : "0 0 12px rgba(255, 93, 93, 0.6)";
  elements.wsStatus.textContent = online ? "Online" : "Offline";
}

export function setCameraStatus(elements, live) {
  elements.cameraStatus.textContent = live ? "LIVE" : "OFFLINE";
  elements.cameraStatus.style.color = live ? "#3dd6f5" : "#ff5d5d";
}

export function setMode(elements, mode) {
  const isJoystick = mode === "joystick";
  elements.modeJoystick.classList.toggle("active", isJoystick);
  elements.modeButtons.classList.toggle("active", !isJoystick);
  elements.joystickZone.style.display = isJoystick ? "block" : "none";
  elements.buttonZone.style.display = isJoystick ? "none" : "flex";
}

export function updateFlashUI(elements, level) {
  const percent = Math.round((level / 255) * 100);
  elements.brightnessSlider.value = percent;
  elements.brightnessSlider.style.setProperty("--slider-value", percent + "%");
}

export function bindHoldButtons(buttons, onPress, onRelease) {
  buttons.forEach((button) => {
    let repeatId = null;

    const stop = () => {
      if (repeatId) {
        clearInterval(repeatId);
        repeatId = null;
      }
      onRelease(button.dataset.dir);
    };

    button.addEventListener("pointerdown", (event) => {
      event.preventDefault();
      onPress(button.dataset.dir);
      repeatId = setInterval(() => onPress(button.dataset.dir), 120);
    });

    button.addEventListener("pointerup", stop);
    button.addEventListener("pointercancel", stop);
    button.addEventListener("pointerleave", stop);
  });
}
