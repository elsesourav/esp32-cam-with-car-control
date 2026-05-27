export function initUI() {
  const elements = {
    wsDot: document.getElementById("ws-dot"),
    wsStatus: document.getElementById("ws-status"),
    cameraStatus: document.getElementById("camera-status"),
    cameraStream: document.getElementById("camera-stream"),
    cameraPlaceholder: document.getElementById("camera-placeholder"),
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
    fwdPowerGroup: document.querySelectorAll("#fwd-power-group .seg-btn"),
    bwdPowerGroup: document.querySelectorAll("#bwd-power-group .seg-btn"),
    popup: document.getElementById("confirm-popup"),
    popupCancel: document.getElementById("popup-cancel"),
    popupConfirm: document.getElementById("popup-confirm"),
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
  if (live) {
    elements.cameraStream.style.display = "block";
    elements.cameraPlaceholder.style.display = "none";
  } else {
    elements.cameraStream.style.display = "none";
    elements.cameraPlaceholder.style.display = "flex";
  }
}

export function setMode(elements, mode) {
  const isJoystick = mode === "joystick";
  elements.modeJoystick.classList.toggle("active", isJoystick);
  elements.modeButtons.classList.toggle("active", !isJoystick);
  elements.joystickZone.style.display = isJoystick ? "block" : "none";
  elements.buttonZone.style.display = isJoystick ? "none" : "flex";
}

export function updateCustomSlider(slider, percent) {
  slider.style.setProperty("--slider-value", percent + "%");
}

export function showConfirmPopup(elements, onConfirm) {
  elements.popup.classList.remove("hidden");
  // Force browser reflow to enable transition
  void elements.popup.offsetWidth;
  elements.popup.classList.add("show");

  const cleanup = () => {
    elements.popup.classList.remove("show");
    setTimeout(() => {
      elements.popup.classList.add("hidden");
    }, 250);
    elements.popupConfirm.removeEventListener("click", confirmHandler);
    elements.popupCancel.removeEventListener("click", cancelHandler);
  };

  const confirmHandler = () => { cleanup(); onConfirm(); };
  const cancelHandler = () => { cleanup(); };

  elements.popupConfirm.addEventListener("click", confirmHandler);
  elements.popupCancel.addEventListener("click", cancelHandler);
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
