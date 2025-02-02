export class WebSocketService {
  private ws: WebSocket | null = null;
  private messageHandlers: ((data: any) => void)[] = [];
  private serverUrl: string = '';

  constructor() {
    this.init();
  }

  private async init() {
    try {
      // Usar la IP y puerto correctos
      const response = await fetch(`http://${window.location.hostname}:3000/api/server-info`);
      const data = await response.json();
      this.serverUrl = data.wsUrl;
      
      this.connect();
    } catch (error) {
      console.error('Error initializing WebSocket:', error);
      setTimeout(() => this.init(), 3000);
    }
  }

  private connect() {
    if (!this.serverUrl) return;
    
    this.ws = new WebSocket(this.serverUrl);

    this.ws.onmessage = async (event) => {
      try {
        let data;
        if (event.data instanceof Blob) {
          // Si es un Blob, convertirlo a texto
          const text = await event.data.text();
          data = JSON.parse(text);
        } else {
          // Si ya es texto
          data = JSON.parse(event.data);
        }
        this.messageHandlers.forEach(handler => handler(data));
      } catch (error) {
        console.error('Error procesando mensaje WebSocket:', error);
      }
    };

    this.ws.onclose = () => {
      console.log('WebSocket desconectado. Reconectando...');
      setTimeout(() => this.connect(), 3000);
    };

    this.ws.onerror = (error) => {
      console.error('Error en WebSocket:', error);
    };
  }

  public addMessageHandler(handler: (data: any) => void) {
    this.messageHandlers.push(handler);
  }

  public sendMessage(message: any) {
    if (this.ws?.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(message));
    } else {
      console.warn('WebSocket no está conectado. Mensaje no enviado:', message);
    }
  }
}

export const wsService = new WebSocketService(); 