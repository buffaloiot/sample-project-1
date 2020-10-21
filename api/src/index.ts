import app from './application';
import log from './log';
import * as env from './env';
const serverPort = env.get('PORT');

app.listen(serverPort, () => {
  return log.info(`server is listening on ${serverPort}`);
});
