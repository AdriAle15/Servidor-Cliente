import React, { useState, useEffect } from 'react';
import { Device } from '../types/device';
import { X } from 'lucide-react';

interface ConfigurationModalProps {
  isOpen: boolean;
  onClose: () => void;
  onSave: (device: Device) => void;
  device: Device;
}

interface FormData {
  name: string;
  identifier: string;  // Nueva variable para identificar el dispositivo
  type: 'button';      // Solo tipo botón
}

export const ConfigurationModal: React.FC<ConfigurationModalProps> = ({
  isOpen,
  onClose,
  onSave,
  device,
}) => {
  const [formData, setFormData] = useState<FormData>({
    name: '',
    identifier: '',
    type: 'button'
  });

  useEffect(() => {
    if (device) {
      setFormData({
        name: device.name || '',
        identifier: device.identifier || '',
        type: 'button'
      });
    }
  }, [device]);

  if (!isOpen) return null;

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    onSave({
      ...device,
      ...formData,
      status: 'configured',
      widgetType: 'button'  // Siempre será tipo botón
    });
    onClose();
  };

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
      <div className="bg-white rounded-lg p-6 w-full max-w-md">
        <div className="flex justify-between items-center mb-4">
          <h2 className="text-xl font-semibold">Configurar Dispositivo</h2>
          <button
            onClick={onClose}
            className="text-gray-500 hover:text-gray-700"
          >
            <X className="w-6 h-6" />
          </button>
        </div>

        <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-1">
              Nombre del Dispositivo
            </label>
            <input
              type="text"
              value={formData.name}
              onChange={(e) => setFormData({ ...formData, name: e.target.value })}
              className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              required
              placeholder="Ej: Luz Sala"
            />
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-700 mb-1">
              Identificador
            </label>
            <input
              type="text"
              value={formData.identifier}
              onChange={(e) => setFormData({ ...formData, identifier: e.target.value })}
              className="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500"
              required
              placeholder="Ej: luz_sala_1"
            />
            <p className="mt-1 text-sm text-gray-500">
              Un identificador único para el dispositivo (sin espacios)
            </p>
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-700 mb-1">
              Tipo de Dispositivo
            </label>
            <div className="px-3 py-2 border border-gray-300 rounded-md bg-gray-50 text-gray-600">
              Botón de Control
            </div>
            <p className="mt-1 text-sm text-gray-500">
              Este dispositivo se controlará mediante un botón de encendido/apagado
            </p>
          </div>

          <div className="flex justify-end gap-2 mt-6">
            <button
              type="button"
              onClick={onClose}
              className="px-4 py-2 text-sm font-medium text-gray-700 bg-gray-100 rounded-md hover:bg-gray-200"
            >
              Cancelar
            </button>
            <button
              type="submit"
              className="px-4 py-2 text-sm font-medium text-white bg-blue-600 rounded-md hover:bg-blue-700"
            >
              Guardar
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};