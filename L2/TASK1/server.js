const express = require('express');
const bodyParser = require('body-parser');

const Helper = require('./helpers/helper');
const Logger = require('./helpers/logger')

const app = express();
const helper = new Helper();
const logger = new Logger();

let clientInstance = 0;
module.exports = (config) => {

  app.use(bodyParser.json());

  app.get('/api/v1/time', (req, res) => {
    const { time } = req.query;
    let correction = helper.getTime() - parseInt(time);

    res.json({ correction });
  });

  app.get('/api/v1/info', (req, res) => {
    let result = {
      status: (clientInstance === 1) ? 'работает' : 'остановлен',
      time: helper.getTime()
    }

    res.json(result)
  });

  app.get('/api/v1/client', (req, res) => {
    const { stop } = req.query;
    if (!logger.exist()) logger.create('client');

    let timerId
    if (clientInstance < 1 && !stop) {
      logger.print('Сервис начал работу')
      clientInstance = 1;

      timerId = setTimeout(async function tick() {
        helper.correctTime(config.port.server, logger)
        if (clientInstance !== 0) timerId = setTimeout(tick, 2100);
      }, 2100);
    } else {
      if (stop) {
        if (clientInstance === 0) {
          logger.print('Сервис остановлен')
        } else {
          logger.print('Сервис будет остановлен')
          clientInstance = 0
          clearTimeout(timerId)
        }
      } else {
        logger.print('Сервис уже работает')
      }
    }

    res.send({ status: 'ok' })
  })

  app.get('/api/v1/server', async (req, res) => {
    if (!logger.exist()) logger.create('server');
    
    helper.startTime();
    logger.print('Центральный сервис начал опрос')

    let timer = setTimeout(async function tick() {
      helper.printClientInfo(config.port.client1, logger);
      helper.printClientInfo(config.port.client2, logger);

      timer = setTimeout(tick, 4000);
    }, 4000);

    res.json({ status: 'ok' });
  })

  return app;
};