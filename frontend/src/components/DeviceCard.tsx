import React from 'react';
import { Device } from '../types/device';
import { Settings, AlertCircle } from 'lucide-react';

interface DeviceCardProps {
  device: Device;
  onConfigure: (device: Device) => void;
}

export const DeviceCard: React.FC<DeviceCardProps> = ({ device, onConfigure }) => {
  const isUnconfigured = device.status === 'unconfigured';

  return (
    <div className={`bg-white rounded-xl shadow-lg p-6 transition-all hover:shadow-xl ${
      isUnconfigured ? 'border-2 border-dashed border-blue-300' : ''
    }`}>
      <div className="flex justify-between items-start mb-4">
        <div>
          <h3 className="font-semibold text-gray-900">
            {device.name || `Dispositivo ${device.id}`}
          </h3>
          <p className="text-sm text-gray-500">{device.ip}</p>
        </div>
        <div className="flex items-center gap-2">
          {isUnconfigured && (
            <AlertCircle className="w-5 h-5 text-amber-500" />
          )}
        </div>
      </div>

      <div className="mb-4">
        {isUnconfigured ? (
          <p className="text-sm text-amber-600">
            Este dispositivo necesita ser configurado
          </p>
        ) : (
          <div className="text-sm text-gray-600">
            <p>Tipo: {device.type}</p>
            <p>Widget: {device.widgetType}</p>
          </div>
        )}
      </div>

      <div className="flex justify-end">
        <button
          onClick={() => onConfigure(device)}
          className="flex items-center gap-2 px-4 py-2 text-sm font-medium text-white bg-blue-600 rounded-lg hover:bg-blue-700 transition-colors"
        >
          <Settings className="w-4 h-4" />
          Configurar
        </button>
      </div>
    </div>
  );
};