import React, { useState, useEffect } from 'react';
import { Device } from './types/device';
import { discoveredDevices } from './data/discoveredDevices';
import { DeviceCard } from './components/DeviceCard';
import { ConfigurationModal } from './components/ConfigurationModal';
import { GraphWidget } from './components/widgets/GraphWidget';
import { GaugeWidget } from './components/widgets/GaugeWidget';
import { IndicatorWidget } from './components/widgets/IndicatorWidget';
import { CameraWidget } from './components/widgets/CameraWidget';
import { wsService } from './services/websocket.service';

interface NetworkIP {
  name: string;
  ip: string;
  wsUrl: string;
}

interface ServerInfo {
  ip: string;
  httpPort: number;
  wsPort: number;
  wsUrl: string;
  availableIPs: NetworkIP[];
}

function App() {
  const [devices, setDevices] = useState<Device[]>(discoveredDevices);
  const [isConfigModalOpen, setIsConfigModalOpen] = useState(false);
  const [selectedDevice, setSelectedDevice] = useState<Device | undefined>();
  const [serverInfo, setServerInfo] = useState<ServerInfo | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [baseUrl, setBaseUrl] = useState<string>('');

  useEffect(() => {
    wsService.addMessageHandler((data) => {
      console.log('WebSocket mensaje recibido:', data);
      
      if (data.type === 'device_connected') {
        console.log('Dispositivo conectado:', data);
        setDevices(prevDevices => {
          // Verificar si el dispositivo ya existe
          const exists = prevDevices.some(
            d => d.identifier === data.identifier && d.ip === data.ip
          );

          if (!exists) {
            // Agregar nuevo dispositivo
            const newDevice: Device = {
              id: Date.now().toString(),
              name: `LED ${data.identifier.split('_')[1]}`,
              identifier: data.identifier,
              type: 'button',
              ip: data.ip,
              status: 'unconfigured',
              data: {
                state: 'off'
              }
            };
            console.log('Agregando nuevo dispositivo:', newDevice);
            return [...prevDevices, newDevice];
          }
          return prevDevices;
        });
      }
      else if (data.type === 'state_update') {
        console.log('Actualización de estado:', data);
        setDevices(prevDevices => 
          prevDevices.map(device => {
            if (device.identifier === data.identifier) {
              console.log('Actualizando dispositivo:', device.identifier);
              return { ...device, data: { state: data.state } };
            }
            return device;
          })
        );
      }
    });

    // Determinar la URL base usando window.location.hostname
    const url = `http://${window.location.hostname}:3000`;
    setBaseUrl(url);

    // Obtener información del servidor usando la URL dinámica
    fetch(`${url}/api/server-info`)
      .then(res => {
        if (!res.ok) {
          throw new Error(`HTTP error! status: ${res.status}`);
        }
        return res.json();
      })
      .then(data => {
        setServerInfo(data);
        console.log('Server Info:', data);
        setError(null);
      })
      .catch(err => {
        console.error('Error fetching server info:', err);
        setError(err.message);
      });
  }, []);

  const handleConfigureDevice = (device: Device) => {
    setSelectedDevice(device);
    setIsConfigModalOpen(true);
  };

  const handleSaveConfiguration = (updatedDevice: Device) => {
    setDevices(devices.map(device => 
      device.id === updatedDevice.id ? updatedDevice : device
    ));
  };

  const unconfiguredDevices = devices.filter(d => d.status === 'unconfigured');
  const configuredDevices = devices.filter(d => d.status === 'configured');

  const renderWidget = (device: Device) => {
    return (
      <button
        className="w-full px-4 py-2 text-sm font-medium text-white bg-blue-600 rounded-md hover:bg-blue-700"
        onClick={() => {
          const newState = device.data?.state === 'on' ? 'off' : 'on';
          wsService.sendMessage({
            type: 'toggle_device',
            identifier: device.identifier,
            state: newState
          });
        }}
      >
        {device.data?.state === 'on' ? 'Apagar' : 'Encender'}
      </button>
    );
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 to-indigo-50 p-6">
      <div className="max-w-7xl mx-auto">
        {/* Banner de IP */}
        <div className="bg-blue-600 text-white px-6 py-3 rounded-lg mb-6 text-center shadow-lg">
          <h2 className="text-2xl font-bold">IP del Servidor</h2>
          <p className="text-4xl font-mono mt-2">
            {serverInfo ? serverInfo.ip : 'Conectando...'}
          </p>
          <p className="text-sm mt-2 text-blue-100">
            Usa esta IP para conectar tus dispositivos ESP32
          </p>
          {error && (
            <div className="mt-2 bg-red-500 text-white p-2 rounded">
              Error: {error}
            </div>
          )}
        </div>

        <div className="mb-8">
          <h1 className="text-3xl font-bold text-gray-900">Panel de Control IoT</h1>
          <p className="text-gray-600">Gestiona tus dispositivos conectados</p>
        </div>

        {devices.length === 0 ? (
          <div className="text-center py-12">
            <p className="text-gray-500 text-lg">
              No hay dispositivos conectados en este momento
            </p>
            <p className="text-gray-400 text-sm mt-2">
              Los dispositivos aparecerán aquí cuando se conecten a la red
            </p>
          </div>
        ) : (
          <div className="grid grid-cols-1 lg:grid-cols-4 gap-6">
            {/* Lista de dispositivos */}
            <div className="lg:col-span-1 bg-white rounded-xl shadow-lg p-6">
              <h2 className="text-xl font-semibold text-gray-800 mb-4">
                Dispositivos
              </h2>
              <div className="space-y-4">
                {devices.map(device => (
                  <div
                    key={device.id}
                    className={`p-4 rounded-lg transition-colors ${
                      device.status === 'unconfigured'
                        ? 'bg-amber-50 border-2 border-dashed border-amber-200'
                        : 'bg-gray-50'
                    }`}
                  >
                    <div className="flex items-center justify-between">
                      <div>
                        <h3 className="font-medium text-gray-900">
                          {device.name || `Dispositivo ${device.id}`}
                        </h3>
                        <p className="text-sm text-gray-500">{device.ip}</p>
                      </div>
                      <button
                        onClick={() => handleConfigureDevice(device)}
                        className="text-blue-600 hover:text-blue-800 text-sm font-medium"
                      >
                        {device.status === 'unconfigured' ? 'Configurar' : 'Editar'}
                      </button>
                    </div>
                  </div>
                ))}
              </div>
            </div>

            {/* Dashboard */}
            <div className="lg:col-span-3">
              <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                {configuredDevices.map(device => (
                  <div key={device.id} className="bg-white rounded-xl shadow-lg p-6">
                    <div className="mb-4">
                      <h3 className="font-semibold text-gray-900">{device.name}</h3>
                      <p className="text-sm text-gray-500">{device.type}</p>
                    </div>
                    {renderWidget(device)}
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}
      </div>

      {selectedDevice && (
        <ConfigurationModal
          isOpen={isConfigModalOpen}
          onClose={() => {
            setIsConfigModalOpen(false);
            setSelectedDevice(undefined);
          }}
          onSave={handleSaveConfiguration}
          device={selectedDevice}
        />
      )}
    </div>
  );
}

export default App;