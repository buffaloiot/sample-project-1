
// import * as bodyParser from 'body-parser';
import * as cors from 'cors';
import * as bodyParser from 'body-parser';
import * as express from 'express';
import { resolve } from 'path';
import { initSwaggerMiddlware } from './middlewares/swagger';
import * as env from './env';
import { inOutLogger } from './log';
import * as cls from './lib/cls';
import { getCorsOptions } from './cors';
import * as Pool from 'pg-pool';

env.checkEnv();
const app = express();
export default app;
app.use(cors(getCorsOptions()));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true}));
app.use(cls.setRequestId);
app.use(inOutLogger);

const dbUrl = new URL(env.get('PG_CONNECT_STRING'));
app.locals.db = new Pool({
  user: dbUrl.username,
  password: dbUrl.password,
  host: dbUrl.hostname,
  port: dbUrl.port,
  database: dbUrl.pathname.split('/')[1],
  ssl: false,
  max: 20,
  idleTimeoutMillis: 1000,
  connectionTimeoutMillis: 1000,
  maxUses: 7500
});

initSwaggerMiddlware(app, resolve(__dirname), () => {
  app.use(function (err, req: express.Request, res: express.Response, next) {
    if (err) {
      const errStr = err.message || err.toString();
      const errMsg = { message: errStr, extra: err };
      if (res.statusCode < 400) {
        res.status(500);
      }
      res.json(errMsg);
    }
  });

});
