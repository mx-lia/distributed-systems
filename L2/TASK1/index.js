const config = require('./config')
const server = require('./server')(config);
const portscanner = require('portscanner');

portscanner.findAPortNotInUse([
  config.port.server, 
  config.port.client1, 
  config.port.client2
], function(error, port) {
  server.listen(port, () => { console.log(`Server listening at ${port}`);})    
})