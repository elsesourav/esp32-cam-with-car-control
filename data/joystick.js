export function createJoystick(zone, knob, { onMove, onEnd }) {
  let active = false;
  let origin = { x: 0, y: 0 };
  let maxRadius = 1;

  function setKnob(x, y) {
    knob.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;
  }

  function updateFromPointer(event) {
    const rect = zone.getBoundingClientRect();
    const dx = event.clientX - rect.left - origin.x;
    const dy = event.clientY - rect.top - origin.y;
    const distance = Math.hypot(dx, dy);
    const clamped = Math.min(distance, maxRadius);
    const angle = Math.atan2(dy, dx);
    const x = Math.cos(angle) * clamped;
    const y = Math.sin(angle) * clamped;

    setKnob(x, y);

    const normX = Math.round((x / maxRadius) * 100);
    const normY = -Math.round((y / maxRadius) * 100);
    onMove({ x: normX, y: normY });
  }

  function pointerDown(event) {
    active = true;
    const rect = zone.getBoundingClientRect();
    origin = { x: rect.width / 2, y: rect.height / 2 };
    maxRadius = Math.min(rect.width, rect.height) * 0.35;
    zone.setPointerCapture(event.pointerId);
    updateFromPointer(event);
  }

  function pointerMove(event) {
    if (!active) {
      return;
    }
    updateFromPointer(event);
  }

  function pointerUp(event) {
    if (!active) {
      return;
    }
    active = false;
    zone.releasePointerCapture(event.pointerId);
    setKnob(0, 0);
    onEnd();
  }

  zone.addEventListener("pointerdown", pointerDown);
  zone.addEventListener("pointermove", pointerMove);
  zone.addEventListener("pointerup", pointerUp);
  zone.addEventListener("pointercancel", pointerUp);

  setKnob(0, 0);
}
