export class WsClient {
  constructor(url) {
    this.url = url;
    this.socket = null;
    this.isConnected = false;
    this.retryMs = 800;
    this.maxRetryMs = 5000;
    this.onStatus = () => {};
    this.onMessage = () => {};
  }

  connect() {
    if (
      this.socket &&
      (this.socket.readyState === WebSocket.OPEN ||
        this.socket.readyState === WebSocket.CONNECTING)
    ) {
      return;
    }

    this.socket = new WebSocket(this.url);
    this.socket.addEventListener("open", () => {
      this.isConnected = true;
      this.retryMs = 800;
      this.onStatus(true);
    });

    this.socket.addEventListener("message", (event) => {
      this.onMessage(event.data);
    });

    this.socket.addEventListener("close", () => {
      this.isConnected = false;
      this.onStatus(false);
      this.scheduleReconnect();
    });

    this.socket.addEventListener("error", () => {
      this.isConnected = false;
      this.onStatus(false);
      this.scheduleReconnect();
    });
  }

  scheduleReconnect() {
    setTimeout(() => {
      this.retryMs = Math.min(this.retryMs * 1.4, this.maxRetryMs);
      this.connect();
    }, this.retryMs);
  }

  send(payload) {
    if (this.socket && this.socket.readyState === WebSocket.OPEN) {
      this.socket.send(payload);
    }
  }
}

export function buildMessage(type, data) {
  const params = new URLSearchParams({ type });
  Object.entries(data).forEach(([key, value]) => {
    params.set(key, String(value));
  });
  return params.toString();
}
