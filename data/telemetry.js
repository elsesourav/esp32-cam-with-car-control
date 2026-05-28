// telemetry.js — MPU6050 telemetry display module.
// Updates the telemetry UI section with data received via WebSocket.

export function updateTelemetry(elements, data) {
  if (!elements.telemetrySection) return;

  if (elements.telX) elements.telX.textContent = Number(data.x).toFixed(2);
  if (elements.telY) elements.telY.textContent = Number(data.y).toFixed(2);
  if (elements.telZ) elements.telZ.textContent = Number(data.z).toFixed(2);
  if (elements.telAccel) elements.telAccel.textContent = Number(data.a).toFixed(2);
  if (elements.telTilt) elements.telTilt.textContent = data.tilt || "—";
  if (elements.telState) elements.telState.textContent = data.state || "—";

  const connected = data.conn === "1" || data.conn === 1;
  if (elements.sensorDot) {
    elements.sensorDot.style.background = connected ? "#3dd6f5" : "#ff5d5d";
    elements.sensorDot.style.boxShadow = connected
      ? "0 0 12px rgba(61, 214, 245, 0.6)"
      : "0 0 12px rgba(255, 93, 93, 0.6)";
  }
  if (elements.sensorStatus) {
    elements.sensorStatus.textContent = connected ? "Connected" : "Disconnected";
  }
}
