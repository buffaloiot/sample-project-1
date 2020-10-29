import { Request, Response } from 'express';
import { TDebug } from '../log';
const log = require('debug-level').log('app:src:controllers:getReadings')

const debug = new TDebug('app:src:controllers:getReadings');

export async function getReadings(req: Request, res: Response): Promise<any> {
    const client = await req.app.locals.db.connect()
    try {
      debug.info('query params', req.query)
      const { location, device_type, device_id, sensor, start_time, end_time, min_value, max_value } = req.query
      const params = [ location, device_type, device_id, sensor, start_time, end_time, min_value, max_value ]
      const result = await client.query('select * from get_readings($1,$2,$3,$4,$5,$6,$7,$8)', params)
      debug.debug('result', result)
      res.send(JSON.stringify(result.rows))
    } catch(e) {
      log.error(e)
      res.status(500).send(JSON.stringify({message:e.message}))
    } finally {
      client.release()
    }
}
