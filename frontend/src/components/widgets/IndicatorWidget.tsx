import React from 'react';
import { Device } from '../../types/device';
import { Power } from 'lucide-react';

interface IndicatorWidgetProps {
  device: Device;
}

export const IndicatorWidget: React.FC<IndicatorWidgetProps> = ({ device }) => {
  const isOn = device.data?.state === 'on';

  return (
    <div className="flex items-center justify-center">
      <div
        className={`w-16 h-16 rounded-full flex items-center justify-center transition-colors ${
          isOn ? 'bg-green-500' : 'bg-gray-300'
        }`}
      >
        <Power className={`w-8 h-8 ${isOn ? 'text-white' : 'text-gray-600'}`} />
      </div>
    </div>
  );
};