//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// WebRTC echo test
//

import { ConsoleLogger } from './console_logger';
import { EchoTransport } from './echo_transport';
import { HTTPServer, HTTPServerPrefix, ProcessJsonResult } from './http';
import { parseSDP, serializeSDP, SDP, MediaDescriptor } from './sdp';
import * as os from 'os';
import { webrtcProcessSDP } from './webrtc_sdp';
const echob = require('../build/Release/echo_backend');

const prefixes: HTTPServerPrefix[] = [
    {method: 'POST', prefix: '/echo-api/offer', action: {type: 'process-json', handler: sdpOffer}},
    {method: 'POST', prefix: '/echo-api/ice-candidate', action: {type: 'process-json', handler: iceCandidate}},
    {method: 'GET', prefix: '/', action: {type: 'proxy', target: 'http://localhost:9000'}},
];

const logger = new ConsoleLogger;
const http = new HTTPServer(logger, prefixes);

http.listen(8000);

async function sdpOffer(req: object): Promise<ProcessJsonResult> {
    const r = parseSDP((req as any).sdp);
    if (r.type === 'error') {
        logger.warn(`SDP parse error: ${r.description}`);
        return {
            result: 'error',
            code: 400,
            description: 'SDP parse error'
        };
    }
    const answerSDP = generateAnswerSDP(r.obj);
    const remoteInfo = webrtcProcessSDP(r.obj);
    const localInfo = webrtcProcessSDP(answerSDP);
    const remoteUfrag = remoteInfo.transports[0].ice.ufrag;
    const localUfrag = localInfo.transports[0].ice.ufrag;
    const localPassword = localInfo.transports[0].ice.pwd;

    const t = new EchoTransport(`${remoteUfrag!}:${localUfrag!}`, localPassword!);
    const addr = await t.start();
    if (addr.address === '0.0.0.0') {
        const candidates = collectNonLoopbackAddresses(addr.family)
            .map((ifaddr) => `candidate:1 1 UDP 1 ${ifaddr} ${addr.port} typ host`);
        return {
            result: 'ok',
            answer: {
                candidates,
                sdp: serializeSDP(answerSDP)
            }
        };
    }
    return {
        result: 'error',
        code: 501,
        description: 'Not supported yet'
    };
}

async function iceCandidate(req: object): Promise<ProcessJsonResult> {
    return {
        result: 'ok',
        answer: echob.ice.candidate_parse((req as any).candidate)
    };
}

function collectNonLoopbackAddresses(family: string): string[] {
    let result: string[] = [];
    const ifaces = os.networkInterfaces();
    for (let name in ifaces) {
        ifaces[name]?.filter((iface) => iface.family === family && !iface.internal)
            .forEach(({address}) => {
                result.push(address);
            });
    }
    return result;
}

function generateAnswerSDP(offerSDP: SDP): SDP {
    return {
        v: 0,
        origin: {
            username: "-",
            sess: {
                id: "0",
                version: "0"
            },
            net: "IN",
            address: {
                type: "IP4",
                value: "127.0.0.1"
            }
        },
        name: "-",
        emails: [],
        phones: [],
        time: ["0 0"],
        bandwidth: [],
        attrs: [...offerSDP.attrs],
        media: offerSDP.media.map((md) => generateAnswerMedia(md))
    };
}

function generateAnswerMedia(md: MediaDescriptor): MediaDescriptor {
    return {
        media: md.media,
        proto: md.proto,
        port: md.port,
        formats: md.formats,
        bandwidth: [],
        connection: [],
        attrs: md.attrs.map((v) => {
            if (v.startsWith('setup:')) {
                switch (v.split(':')[1]) {
                    case 'actpass': return 'setup:passive';
                    case 'passive': return 'setup:active';
                    case 'active':  return 'setup:passive';
                }
            }
            return v;
        })
    };
}
