import React from 'react';
import { Device } from '../../types/device';
import { Video } from 'lucide-react';

interface CameraWidgetProps {
  device: Device;
}

export const CameraWidget: React.FC<CameraWidgetProps> = ({ device }) => {
  const isRecording = device.data?.state === 'recording';

  return (
    <div className="relative w-full h-48 bg-gray-900 rounded-lg overflow-hidden flex items-center justify-center">
      {isRecording && (
        <div className="absolute top-4 right-4 flex items-center gap-2">
          <div className="w-3 h-3 rounded-full bg-red-500 animate-pulse" />
          <span className="text-xs text-white">REC</span>
        </div>
      )}
      <Video className="w-12 h-12 text-white opacity-50" />
    </div>
  );
};