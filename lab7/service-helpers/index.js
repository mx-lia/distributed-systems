const dgram = require('dgram');
const fs = require('fs');
const config = require('./../config');

const timeServiceHandlers = {
  'time': getTimeHandler,
  'ready': checkServerReady,
  'ready-ok': coordinatorIsReady,
  'init-coordinator': initCoordinator,
  're-init-coordinator': reInitCoordinator,
};

const coordinatorFilePath = '../coordinator.json';

const self = module.exports = {

  initTimeServer: serverId => {
    const server = dgram.createSocket('udp4');
    server.ready = false;

    const timeServerConfig = config.timeService[`127.0.0.${serverId + 1}`];

    server.on('message', (message, client) => {
      console.log(`Server-${serverId} got: "${message}" from ${client.address}:${client.port}`);
      const handler = timeServiceHandlers[message];
      if (handler) {
        handler(server, client);
      }
    });

    server.on('error', err => {
      console.log(`Server-${serverId} error:\n${err.stack}`);
      server.close();
    });

    server.on('listening', () => {
      server.ready = true;
      server.setBroadcast(true);
      const address = server.address();

      console.log(`Server-${serverId} is listening to ${address.address}:${address.port}`);
      const coordinator = self.getCoordinator();
      if (coordinator.host === address.address) {
        console.log('Coordination...');
      } else {
        const myRang = config.timeService[server.address().address].rang;
        const clientRang = config.timeService[coordinator.host].rang;
        if (myRang > clientRang) {
          console.log(`My rang (${myRang}) is higher`);
          server.send('re-init-coordinator', config.timeService.port, config.timeService.broadcastHost);
        }
      }
    });

    server.bind(config.timeService.port, timeServerConfig.host);

    setInterval(
      self.checkCoordinatorAvailable,
      config.timeService.checkCoordinatorInterval,
      server);

    return server;
  },

  getCoordinator: () => {
    return JSON.parse(fs.readFileSync(coordinatorFilePath).toString());
  },

  setCoordinator: (host, port) => {
    fs.writeFile(
      coordinatorFilePath,
      JSON.stringify({
        host,
        port,
      }, null, '  '),
      err => {
        if (err) {
          console.log(`Error while saving current coordinator`);
        }
      });
  },

  checkCoordinatorAvailable: server => {
    const coordinator = self.getCoordinator();
    if (coordinator.host === server.address().address) {
      return;
    }

    server.coordinatorReady = false;
    console.log('Check coordinator is available');
    if (!coordinator) {
      console.log('Coordinator is not available');
      return server.send(
        're-init-coordinator',
        config.timeService.port,
        config.timeService.broadcastHost);
    }
    if (server.address().address !== coordinator.host) {
      server.coordinator = coordinator;
      server.send('ready', coordinator.port, coordinator.host);
      self.recheckCoordinatorAvailable(server, 1);
    }
  },

  recheckCoordinatorAvailable: (server, attempt) => {
    setTimeout(() => {
      if (!server.coordinatorReady && attempt < config.timeService.checkCoordinatorAttempts) {
        console.log('Recheck coordinator is available');
        server.send('ready', server.coordinator.port, server.coordinator.host);
        self.recheckCoordinatorAvailable(server, ++attempt);
      } else if (server.coordinatorReady) {
        console.log('Coordinator is available');
      } else {
        console.log('Coordinator is not available');
        server.send(
          're-init-coordinator',
          config.timeService.port,
          config.timeService.broadcastHost);
      }
    }, 300);
  },
};

function getTimeHandler(server, client) {
  if (server.ready) {
    server.send(new Date().toISOString(), client.port, client.address);
  }
}

function checkServerReady(server, client) {
  if (server.ready) {
    server.send('ready-ok', client.port, client.address);
  }
}

function coordinatorIsReady(server, client) {
  server.coordinatorReady = true;
}

function initCoordinator(server, client) {
  server.ready = true;
  server.coordinatorReady = true;
  server.coordinator = {
    host: client.address,
    port: client.port,
  };
  console.log(`Initial coordinator: ${server.coordinator.host}:${server.coordinator.port}`);
}

function reInitCoordinator(server, client) {
  const myRang = config.timeService[server.address().address].rang;
  const clientRang = config.timeService[client.address].rang;

  console.log(`New coordinator suggestion with rang ${clientRang}`);
  if (myRang > clientRang) {
    console.log(`My rang (${myRang}) is higher`);
    server.send('re-init-coordinator', config.timeService.port, config.timeService.broadcastHost);
  } else {
    console.log('Agree with a new coordinator');
    server.coordinatorReady = true;
    if (server.address().address === client.address) {
      console.log('Coordinating...');
    }
    self.setCoordinator(client.address, client.port);
  }
}
