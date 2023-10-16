//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// WebRTC echo transport
//

import { createSocket, Socket, RemoteInfo } from 'dgram';
import { AddressInfo } from "net";

export class EchoTransport {
    private readonly socket: Socket;
    constructor() {
        this.socket = createSocket('udp4');
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
        console.log(msg);
    }
};



