//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Simple HTTP server
//

import * as http from 'http';
import { URL } from 'url';

import { Logger } from './logger';

export type HTTPServerPrefix = {
    method: string;
    prefix: string;
    action: PrefixAction;
};

type Prefix = HTTPServerPrefix;

type PrefixAction = ProxyAction | ProcessJsonAction;

type ProxyAction = {
    type: 'proxy';
    target: string;
};

export type ProcessJsonResult = ProcessJsonOk | ProcessJsonError;
type ProcessJsonOk = {
    result: 'ok';
    answer: object;
};
type ProcessJsonError = {
    result: 'error';
    code: number;
    description: string;
};

type ProcessJsonHandler = (req: object) => Promise<ProcessJsonResult>;

type ProcessJsonAction = {
    type: 'process-json';
    handler: ProcessJsonHandler;
}

export class HTTPServer {
    private readonly srv: http.Server;
    private readonly prefixes: Prefix[];
    private readonly logger: Logger;
    constructor(logger: Logger, prefixes: Prefix[]) {
        this.logger = logger;
        this.prefixes = prefixes || [];
        this.srv = http.createServer((req, resp) => {
            this.processRequest(req, resp);
        });
    }
    async listen(port: number) {
        this.srv.listen(port);
    }
    private processRequest(req: http.IncomingMessage, res: http.ServerResponse) {
        const maybePrefix = this.prefixes.find((p) => {
            return p.method === req.method
                && req.url?.startsWith(p.prefix);
        });
        if (!maybePrefix) {
            return response(404, res);
        }
        const action = maybePrefix.action;
        this.logger.debug(`found action ${action.type} for URL: ${req.url}`);
        switch (action.type) {
            case 'proxy':
                return doProxy(this.logger, action.target, req, res);
            case 'process-json':
                return doProcessJson(this.logger, action.handler, req, res);
        }
    }
}

function doProxy(logger: Logger, target: string, req: http.IncomingMessage, res: http.ServerResponse) {
    const targetUrl = new URL(target + req.url);
    logger.info(`proxy request to URL: ${targetUrl.toString()}`);
    const options = {
        hostname: targetUrl.hostname,
        port: targetUrl.port || 80,
        path: targetUrl.pathname + targetUrl.search,
        method: req.method,
        headers: req.headers
    };

    const proxy = http.request(options, (targetRes) => {
        res.writeHead(targetRes.statusCode || 500, targetRes.headers);
        targetRes.pipe(res, {end: true});
    });

    proxy.on('error', (err) => {
        logger.warn(`proxy target error occured: ${err.message}`);
        response(500, res);
    });

    req.pipe(proxy, {end: true});

    req.on('error', (err) => {
        logger.warn(`proxy client error occured: ${err.message}`);
        proxy.end();
    });
}

function doProcessJson(logger: Logger, handler: ProcessJsonHandler, req: http.IncomingMessage, res: http.ServerResponse) {
    let data = '';
    if (req.headers['content-type'] !== 'application/json') {
        return response(406, res);
    }
    req.on('data', (chunk) => {
        data += chunk;
    });
    req.on('end', () => {
        try {
            const jreq = JSON.parse(data);
            handler(jreq)
                .then((jresp) => {
                    switch (jresp.result) {
                        case 'ok':
                            res.writeHead(200, {
                                'content-type': 'application/json'
                            });
                            return res.end(JSON.stringify(jresp.answer));
                        case 'error':
                            res.writeHead(jresp.code);
                            return res.end(jresp.description);
                    }
                })
                .catch((err) => {
                    logger.error(`Unexpected exception: ${(err as Error).stack}`);
                    return response(500, res);                
                });
        } catch (err) {
            logger.warn(`Failed to parse JSON payload: ${(err as Error).message}`)
            return response(400, res);
        }
    });
}


function response(code: number, res: http.ServerResponse) {
    res.writeHead(code);
    switch (code) {
        case 400: return res.end('Bad request');
        case 404: return res.end('File not found');
        case 405: return res.end('Method not supported');
        case 406: return res.end('Not acceptable');
        default:  return res.end('Internal server error');
    }
}
