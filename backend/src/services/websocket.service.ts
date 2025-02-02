import WebSocket from 'ws';
import { Device } from '../types/device';
import { DeviceModel } from '../models/device.model';
import http from 'http';

export class WebSocketService {
  private wss: WebSocket.Server;
  private clients: Map<string, WebSocket> = new Map();

  constructor(server: http.Server) {
    this.wss = new WebSocket.Server({ server });
    this.init();
  }

  private init() {
    this.wss.on('connection', (ws: WebSocket, req: http.IncomingMessage) => {
      const clientIp = req.socket.remoteAddress;
      console.log('Nueva conexión WebSocket desde:', clientIp);
      
      // Registrar el dispositivo si es nuevo
      this.handleNewDevice(clientIp, ws);

      ws.on('message', (message: Buffer) => {
        try {
          const messageStr = message.toString();
          console.log('Mensaje recibido:', messageStr);
          
          // Verificar que el mensaje es JSON válido
          const data = JSON.parse(messageStr);
          
          // Retransmitir el mensaje como string a todos los clientes
          this.wss.clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
              client.send(messageStr);
            }
          });
        } catch (error) {
          console.error('Error procesando mensaje:', error);
        }
      });

      ws.on('close', () => {
        console.log('Cliente desconectado:', clientIp);
        this.clients.delete(clientIp);
        this.updateDeviceStatus(clientIp, 'unconfigured');
      });

      ws.on('error', (error) => {
        console.error('Error en conexión WebSocket:', error);
      });
    });
  }

  private async handleNewDevice(ip: string, ws: WebSocket) {
    this.clients.set(ip, ws);
    
    // Buscar o crear dispositivo en la base de datos
    let device = await DeviceModel.findByIp(ip);
    if (!device) {
      device = await DeviceModel.createDevice({
        ip,
        status: 'unconfigured',
        type: 'switch',
        data: { state: 'off' }
      });
    }
  }

  private async handleMessage(ip: string, data: any) {
    if (data.type === 'state_update') {
      await DeviceModel.updateDeviceState(ip, data.state);
      this.broadcastToFrontend({
        type: 'device_update',
        ip,
        state: data.state
      });
    }
  }

  private async updateDeviceStatus(ip: string, status: string) {
    await DeviceModel.updateDeviceStatus(ip, status);
    this.broadcastToFrontend({
      type: 'device_status',
      ip,
      status
    });
  }

  public broadcastToFrontend(data: any) {
    this.wss.clients.forEach(client => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(JSON.stringify(data));
      }
    });
  }

  public sendToDevice(ip: string, message: any) {
    const client = this.clients.get(ip);
    if (client && client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify(message));
    }
  }
} 