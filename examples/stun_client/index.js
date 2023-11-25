
const { Resolver } = require('node:dns');
const { EventEmitter }  = require('events');
const util = require('util');
const stun = require('./build/Release/stun_client.node');
const dgram = require('dgram');

const resolver = new Resolver();
const resolve = util.promisify(resolver.resolve4.bind(resolver));

class UdpSocket extends EventEmitter {
    constructor() {
        super();
        this.sock = dgram.createSocket('udp4');
        this.q = [];
        this.sock.on('message', (msg, rinfo) => {
            this.q.push(msg);
            this.emit('message', msg);
        });
    }
    send(msg, port, addr) {
        this.sock.send(msg, port, addr);
    }
    async receive(timeout) {
        if (this.q.length !== 0) {
            return this.q.shift();
        }
        return new Promise((resolve) => {
            const listener = () => {
                clearTimeout(alarm);
                resolve(this.q.shift());
            };
            let alarm = setTimeout(() => {
                this.removeListener('message', listener);
                resolve(null);
            }, timeout);
            this.once('message', listener);
        });
    }
    close() {
        this.sock.close();
    }
}

async function request(host, port) {
    const udpClient = new stun.ClientUDP({});
    let addr = await resolve(host);
    if (addr.length === 0) {
        throw new Error(`Cannot resolve: ${host}`);
    }
    addr = addr[0];
    const hnd = udpClient.create({
        target: {
            addr,
            port
        }
    });
    const sock = new UdpSocket();
    let result = null;

    while (true) {
        const effect = udpClient.next();
        switch (effect.type) {
        case 'transaction_ok':
            console.log(`Successful: server reflexive: ${effect.result.addr}:${effect.result.port}; rtt: ${effect.rtt_us/1000}ms`);
            sock.close();
            return effect.result;
        case 'transaction_fail':
            console.log(effect);
            console.log(`Failed`);
            break;
        case 'sleep':
            console.log(`Waiting: ${effect.timeout_ms}ms`);
            let rx = await sock.receive(effect.timeout_ms);
            if (rx !== null) {
                udpClient.response(rx);
            }
            break;
        case 'send_data':
            console.log(`Sending: ${effect.message.length} bytes to ${addr}:${port}`);
            sock.send(effect.message, port, addr);
            break;
        case 'idle':
            sock.close();
            return result;
        }
    }
}

let host = 'stun.l.google.com';
let port = 19302;

request(host, port)
    .then(console.log);
