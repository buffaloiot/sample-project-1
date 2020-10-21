import { Router } from 'express';
import { getReadings } from '../controllers/readings';
import { asyncHandler } from '../lib/asyncHandler';
export const readingsGet  = Router().use('/', asyncHandler(getReadings, 'readingsGet'));
