import React from 'react';
import { Device } from '../../types/device';

interface GaugeWidgetProps {
  device: Device;
}

export const GaugeWidget: React.FC<GaugeWidgetProps> = ({ device }) => {
  const temperature = device.data?.temperature || 0;
  const percentage = (temperature / 50) * 100; // Asumiendo un máximo de 50°C
  const rotation = (percentage * 1.8) - 90; // Convertir a grados (-90 a 90)

  return (
    <div className="relative w-32 h-32 mx-auto">
      <div className="absolute inset-0 flex items-center justify-center">
        <div className="w-full h-full bg-gray-200 rounded-full overflow-hidden transform -rotate-90">
          <div
            className="h-full bg-blue-500 transition-all duration-500"
            style={{
              width: '100%',
              transform: `rotate(${rotation}deg)`,
              transformOrigin: '50% 50%',
              background: 'conic-gradient(from 0deg, #3b82f6 0%, transparent 50%)',
            }}
          />
        </div>
        <div className="absolute inset-0 flex items-center justify-center">
          <span className="text-2xl font-bold text-gray-800">
            {temperature}°C
          </span>
        </div>
      </div>
    </div>
  );
};