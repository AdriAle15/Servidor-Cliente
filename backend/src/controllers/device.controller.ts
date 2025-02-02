import { Request, Response } from 'express';
import { DeviceModel } from '../models/device.model';

export class DeviceController {
  static async getAllDevices(req: Request, res: Response) {
    try {
      const devices = await DeviceModel.getAllDevices();
      res.json(devices);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  static async getDeviceById(req: Request, res: Response) {
    try {
      const device = await DeviceModel.getDeviceById(req.params.id);
      if (!device) {
        return res.status(404).json({ error: 'Device not found' });
      }
      res.json(device);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  static async createDevice(req: Request, res: Response) {
    try {
      const device = await DeviceModel.createDevice(req.body);
      res.status(201).json(device);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  static async updateDevice(req: Request, res: Response) {
    try {
      const device = await DeviceModel.updateDevice(req.params.id, req.body);
      if (!device) {
        return res.status(404).json({ error: 'Device not found' });
      }
      res.json(device);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  static async deleteDevice(req: Request, res: Response) {
    try {
      await DeviceModel.deleteDevice(req.params.id);
      res.status(204).send();
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }
} 