import { pool } from '../config/db';
import { Device } from '../types/device';

export class DeviceModel {
  static async getAllDevices() {
    try {
      const result = await pool.query('SELECT * FROM dispositivos_conectados');
      return result.rows;
    } catch (error) {
      throw new Error(`Error getting devices: ${error}`);
    }
  }

  static async getDeviceById(id: string) {
    try {
      const result = await pool.query('SELECT * FROM dispositivos_conectados WHERE id = $1', [id]);
      return result.rows[0];
    } catch (error) {
      throw new Error(`Error getting device: ${error}`);
    }
  }

  static async createDevice(device: Partial<Device>) {
    try {
      const result = await pool.query(
        'INSERT INTO dispositivos_conectados (name, type, ip, status, data) VALUES ($1, $2, $3, $4, $5) RETURNING *',
        [device.name, device.type, device.ip, device.status, device.data || {}]
      );
      return result.rows[0];
    } catch (error) {
      throw new Error(`Error creating device: ${error}`);
    }
  }

  static async updateDevice(id: string, device: Partial<Device>) {
    try {
      const result = await pool.query(
        'UPDATE dispositivos_conectados SET name = $1, type = $2, status = $3 WHERE id = $4 RETURNING *',
        [device.name, device.type, device.status, id]
      );
      return result.rows[0];
    } catch (error) {
      throw new Error(`Error updating device: ${error}`);
    }
  }

  static async deleteDevice(id: string) {
    try {
      await pool.query('DELETE FROM dispositivos_conectados WHERE id = $1', [id]);
      return true;
    } catch (error) {
      throw new Error(`Error deleting device: ${error}`);
    }
  }

  static async findByIp(ip: string) {
    try {
      const result = await pool.query('SELECT * FROM dispositivos_conectados WHERE ip = $1', [ip]);
      return result.rows[0];
    } catch (error) {
      throw new Error(`Error finding device: ${error}`);
    }
  }

  static async updateDeviceState(ip: string, state: 'on' | 'off') {
    try {
      const result = await pool.query(
        'UPDATE dispositivos_conectados SET data = jsonb_set(COALESCE(data, \'{}\'), \'{state}\', $1) WHERE ip = $2 RETURNING *',
        [JSON.stringify(state), ip]
      );
      return result.rows[0];
    } catch (error) {
      throw new Error(`Error updating device state: ${error}`);
    }
  }

  static async updateDeviceStatus(ip: string, status: string) {
    try {
      const result = await pool.query(
        'UPDATE dispositivos_conectados SET status = $1 WHERE ip = $2 RETURNING *',
        [status, ip]
      );
      return result.rows[0];
    } catch (error) {
      throw new Error(`Error updating device status: ${error}`);
    }
  }
} 