import { Router } from 'express';
import { DeviceController } from '../controllers/device.controller';

const router = Router();

router.get('/devices', DeviceController.getAllDevices);
router.get('/devices/:id', DeviceController.getDeviceById);
router.post('/devices', DeviceController.createDevice);
router.put('/devices/:id', DeviceController.updateDevice);
router.delete('/devices/:id', DeviceController.deleteDevice);

export default router; 