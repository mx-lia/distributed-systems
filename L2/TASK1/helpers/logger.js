const winston = require('winston');
const uuidv1 = require('uuid/v1')

class Logger {
  constructor (filename) {
    this.logger = null;     
  }

  exist () {
    return this.logger !== null ? true : false;
  }

  create (filename) {
    this.logger = new (winston.Logger)({
      transports: [
        new (winston.transports.Console)(),
        new (winston.transports.File)({ filename: `${filename}-${uuidv1()}.log` })
      ]
    });
  }

  print (message) {
    this.logger.info(message);
  }
}

module.exports = Logger