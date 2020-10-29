import { Request, Response } from 'express';
import { TDebug } from '../log';
const log = new TDebug('app:src:controllers:getDevices');

export async function getDevices(req: Request, res: Response): Promise<any> {
    const client = await req.app.locals.db.connect()
    try {
      log.info('query params', req.query)
      const { location, device_type, device_id } = req.query
      const params = [ location, device_type, device_id ]
      const result = await client.query('select * from get_devices($1,$2,$3)', params)
      log.debug('result', result)
      res.send(JSON.stringify(result.rows))
    } catch(e) {
      log.error(e)
      res.status(500).send(JSON.stringify({message:e.message}))
    } finally {
      client.release()
    }
}
