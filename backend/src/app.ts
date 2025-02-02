import express from 'express';
import cors from 'cors';
import http from 'http';
import { networkInterfaces } from 'os';
import deviceRoutes from './routes/device.routes';
import { WebSocketService } from './services/websocket.service';

const app = express();
const server = http.createServer(app);

app.use(cors());
app.use(express.json());

// Función para obtener la IP de WiFi
const getWiFiIP = () => {
  const nets = networkInterfaces();
  console.log('Interfaces de red disponibles:', nets);
  
  // Buscar primero en la interfaz WiFi
  if (nets['Wi-Fi']) {
    for (const net of nets['Wi-Fi']) {
      if (net.family === 'IPv4' && !net.internal) {
        console.log('IP WiFi encontrada:', net.address);
        return net.address;
      }
    }
  }

  // Si no encuentra WiFi, buscar en cualquier interfaz no interna
  for (const name of Object.keys(nets)) {
    for (const net of nets[name]) {
      if (net.family === 'IPv4' && !net.internal) {
        console.log('IP alternativa encontrada:', net.address);
        return net.address;
      }
    }
  }
  
  console.log('No se encontró IP, usando localhost');
  return 'localhost';
};

const SERVER_IP = getWiFiIP();
const HTTP_PORT = 3000;
const WS_PORT = 80;

console.log('Servidor configurado con IP:', SERVER_IP);

// Ruta para obtener la IP del servidor
app.get('/api/server-info', (req, res) => {
  res.json({
    ip: SERVER_IP,
    httpPort: HTTP_PORT,
    wsPort: WS_PORT,
    wsUrl: `ws://${SERVER_IP}:${WS_PORT}`,
    availableIPs: [{ 
      name: 'WiFi', 
      ip: SERVER_IP,
      wsUrl: `ws://${SERVER_IP}:${WS_PORT}`
    }]
  });
});

app.use('/api', deviceRoutes);

// Crear servidor HTTP
const httpServer = app.listen(HTTP_PORT, '0.0.0.0', () => {
  console.log(`HTTP Server running on http://${SERVER_IP}:${HTTP_PORT}`);
});

// Crear servidor WebSocket
const wss = new WebSocketService(server);
server.listen(WS_PORT, '0.0.0.0', () => {
  console.log(`WebSocket Server running on ws://${SERVER_IP}:${WS_PORT}`);
});

export { app, server }; 