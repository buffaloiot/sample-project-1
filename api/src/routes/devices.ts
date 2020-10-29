import { Router } from 'express';
import { getDevices } from '../controllers/devices';
import { asyncHandler } from '../lib/asyncHandler';
export const devicesGet  = Router().use('/', asyncHandler(getDevices, 'devicesGet'));
