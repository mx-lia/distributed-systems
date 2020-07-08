const request = require('request')


class Helper {  

  constructor () {
    this.Cs = 0;
  }

  startTime () {
    setInterval(() => {
      this.Cs += 1
    }, 1)
  }

  getTime () {
    return this.Cs;
  }

  correctTime (port, logger) {
    request(`http://localhost:${port}/api/v1/time?time=${this.Cs}`, (error, response, body) => {
      if (error) {
        logger.print(`Центральный сервер ${port} не найден`)
      } else {
        let data = JSON.parse(body);
        this.Cs += parseInt(data.correction);
        logger.print('Сервис обновил свое время')
      }
    }); 
  }

  printClientInfo (port, logger) {
    request(`http://localhost:${port}/api/v1/info`, function (error, response, body) {
      if (error && error.code === 'ECONNREFUSED') {
        logger.print(`Сервис ${port} не найден`)
      } else if (!error) {
        let data = JSON.parse(body);
        logger.print(`Сервис ${port} ${data.status}, Время ${data.time}`)
      }
    });    
  }
}

module.exports = Helper;