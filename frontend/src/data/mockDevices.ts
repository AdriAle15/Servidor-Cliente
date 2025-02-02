import { Device } from '../types/device';

export const mockDevices: Device[] = [
  {
    id: '1',
    name: 'Sensor Temperatura Sala',
    type: 'temperature',
    ip: '192.168.1.100',
    status: 'active',
    data: {
      temperature: 23.5,
      humidity: 45,
    },
  },
  {
    id: '2',
    name: 'Cámara Principal',
    type: 'camera',
    ip: '192.168.1.101',
    status: 'active',
    data: {
      state: 'recording',
    },
  },
  {
    id: '3',
    name: 'Interruptor Inteligente',
    type: 'switch',
    ip: '192.168.1.102',
    status: 'inactive',
    data: {
      state: 'off',
    },
  },
  {
    id: '4',
    name: 'LED',
    type: 'led',
    ip: 'led1',
    status: 'active',
    data: {
      state: 'on',
    },
  },
];