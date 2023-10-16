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
import { parseSDP } from './sdp';

const prefixes: HTTPServerPrefix[] = [
    {method: 'POST', prefix: '/echo-api/offer', action: {type: 'process-json', handler: sdpOffer}},
    {method: 'GET', prefix: '/', action: {type: 'proxy', target: 'http://localhost:9000'}},
];

const logger = new ConsoleLogger;
const http = new HTTPServer(logger, prefixes);

http.listen(8000);

async function sdpOffer(req: object): Promise<ProcessJsonResult> {
    const sdp = parseSDP((req as any).sdp);
    const t = new EchoTransport;
    const addr = await t.start();
    console.log(sdp, addr);
    return {
        result: 'error',
        code: 501,
        description: 'Not supported yet'
    };
}




