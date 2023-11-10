
//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// WebRTC echo transport
//

import { createSocket, Socket, RemoteInfo } from 'dgram';
import { AddressInfo } from "net";
const echob = require('../build/Release/echo_backend');

export class EchoTransport {
    private readonly socket: Socket;
    private readonly stunServer: any;
    constructor(user: string, password: string) {
        this.socket = createSocket('udp4');
        this.stunServer = new echob.stun.StatelessServer();
        this.stunServer.addUser(user, password);
    }
    start() : Promise<AddressInfo> {
        return new Promise((resolve, reject) => {
            this.socket.on('error', reject);
            this.socket.on('listening', () => {
                resolve(this.socket.address());
            });
            this.socket.bind(0);
            this.socket.on('message', (msg, rinfo) => {
                this.receive(msg, rinfo);
            });
        });
    }
    receive(msg: Buffer, rinfo: RemoteInfo) {
        console.log(rinfo);
        try {
            console.log('Request:', echob.stun.message_parse(msg));
        } catch (e) {
        }
        let v = this.stunServer.process(msg, rinfo);
        switch (v.result) {
            case 'ignore':
                if (v.message) {
                    console.log('Ignored STUN message', v.message);
                }
                break;
            case 'respond':
                console.log('Response:', echob.stun.message_parse(v.data));
                this.socket.send(v.data, rinfo.port, rinfo.address);
                break;
        }
    }
};



