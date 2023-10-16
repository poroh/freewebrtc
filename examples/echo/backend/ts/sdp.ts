//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// Basic SDP parser
// This parser is not aware of WebRTC details
//

type ParseSuccess<T> = {
    type: 'success';
    obj: T;
    rest: string;
};

type ParseError = {
    type: 'error';
    description: string;
}

type ParseResult<T> = ParseSuccess<T> | ParseError;

type V = { v: 0 };
type Address = { type: string; value: string };
type Origin = {
    origin: {
        username: string;
        sess: {
            id: string;
            version: string;
        };
        net: string;
        address: Address
    }
};
type SessionName = { name: string; };
type Information = { information?: string };
type Uri = { uri?: string };
type Email = { emails: string[] };
type Phone = { phones: string[] };
type ConnectionType = { net: string; address: Address };
type SessionConnection = { connection?: ConnectionType };

type Bandwidth = { bandwidth: { type: string;  value: string; }[] };
type Time = { time: string[]; };
type Zone = { zone?: string; }
type Key = { key?: string; }
type Attribute = { attrs: string[] };
type Medias = { media: MediaDescriptor[] };
export type MediaDescriptor = Media
    & Information
    & MediaConnection
    & Bandwidth
    & Key
    & Attribute;

type Media = {
    media: string;
    port: number;
    integer?: string;
    proto: string;
    formats: number[];
};

type MediaConnection = {
    connection: ConnectionType[] // grammar on MediaDescriptor level permit many c= ...
};

export type SDP = V
    & Origin
    & SessionName
    & Information
    & Uri
    & Email
    & Phone
    & SessionConnection
    & Bandwidth
    & Time
    & Zone
    & Attribute
    & Medias;

// session-description = proto-version
//                       origin-field
//                       session-name-field
//                       information-field
//                       uri-field
//                       email-fields
//                       phone-fields
//                       connection-field
//                       bandwidth-fields
//                       time-fields
//                       key-field
//                       attribute-fields
//                       media-descriptions
const SDPParsers = [
    parseProto,
    parseOrigin,
    makeFullFieldParser<SessionName>('s', 'session name', (str) => { return { name: str }; }),
    makeFullOptFieldParser<Information>('i', (str) => { return { information: str }; }),
    makeFullOptFieldParser<Uri>('u', (str) => { return { uri: str }; }),
    makeFullMultFieldParser<Email>('e', (strs) => { return { emails: strs }; }),
    makeFullMultFieldParser<Phone>('p', (strs) => { return { phones: strs }; }),
    parseConnection,
    makeParseBandwidth(),
    makeFullMultFieldParser<Time>('t', (strs) => { return { time: strs }; }),
    makeFullOptFieldParser<Zone>('z', (str) => { return { zone: str }; }),
    makeFullOptFieldParser<Key>('k', (str) => { return { key: str }; }),
    makeFullMultFieldParser<Attribute>('a', (strs) => { return { attrs: strs }; }),
    parseMedias
];

export function parseSDP(sdp: string): ParseResult<SDP> {
    const result = parsersReducer(SDPParsers, sdp);
    if (result.type === 'success' && result.rest.length !== 0) {
        return makeParseError(`Not fully parsed: rest: ${result.rest}`);
    }
    return result as ParseResult<SDP>;
}

function makeParseSuccess<T>(obj: T, rest: string): ParseSuccess<T> {
    return {
        type: 'success',
        obj,
        rest
    }
}

function makeParseError(description: string): ParseError {
    return {
        type: 'error',
        description
    };
}

function parseProto(sdp: string): ParseResult<V> {
    const version = 'v=0\r\n';
    if (sdp.startsWith(version)) {
        return makeParseSuccess({v: 0}, sdp.substring(version.length));
    }
    return makeParseError(`protocol version is expected`);
}

function parseOrigin(str: string): ParseResult<Origin> {
    // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
    const result = /^o=(.*) (.*) (.*) (.*) (.*) (.*)\r\n/.exec(str);
    if (!result) {
        return makeParseError(`cannot parse origin: ${str}`);
    }
    return makeParseSuccess({
        origin: {
            username: result[1],
            sess: {
                id: result[2],
                version: result[3]
            },
            net: result[4],
            address: {
                type: result[5],
                value: result[6]
            }
        }
    }, str.substring(result[0].length));
}

function parseConnection(str: string): ParseResult<SessionConnection> {
    // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
    const result = /^c=(.*) (.*) (.*)\r\n/.exec(str);
    if (!result) {
        if (str.startsWith('c=')) {
            return makeParseError(`cannot parse connection: ${str}`);
        } else {
            return makeParseSuccess({}, str);
        }
    }
    return makeParseSuccess({
        connection: {
            net: result[1],
            address: {
                type: result[2],
                value: result[3]
            }
        }
    }, str.substring(result[0].length));
}

function makeParseBandwidth() {
    const mfp = makeFullMultFieldParser('b', (strs) => strs);
    return (str: string): ParseResult<Bandwidth> => {
        const r = mfp(str);
        if (r.type === 'error') {
            return r;
        }
        try {
            const bws = r.obj.map((str: string) => {
                const v = /(.*):.*/.exec(str);
                if (!v) {
                    throw new Error(`failed to parse bandwidth: ${str}`);
                }
                return {type: v[1], value: v[2]};
            });
            return makeParseSuccess({bandwidth: bws}, r.rest);
        } catch (err) {
            return makeParseError((err as Error).message)
        }
    };
}

// media-descriptions =  *( media-field
//                          information-field
//                          *connection-field
//                          bandwidth-fields
//                          key-field
//                          attribute-fields )
function parseMedias(str: string): ParseResult<Medias> {
    let media: MediaDescriptor[] = []
    while (str.length !== 0) {
        const r = parseMediaDescriptor(str);
        if (r.type === 'error') {
            return r;
        }
        media.push(r.obj);
        str = r.rest;
    }
    return makeParseSuccess({media}, "");
}

const MediaDescriptorParsers = [
    parseMedia,
    makeFullOptFieldParser<Information>('i', (str) => { return { information: str }; }),
    parseConnections,
    makeParseBandwidth(),
    makeFullOptFieldParser<Key>('k', (str) => { return { key: str }; }),
    makeFullMultFieldParser<Attribute>('a', (strs) => { return { attrs: strs }; }),
];

function parseMediaDescriptor(str: string): ParseResult<MediaDescriptor> {
    return parsersReducer(MediaDescriptorParsers, str);
}

function parseMedia(str: string): ParseResult<Media> {
    //  media-field = %x6d "=" media SP port ["/" integer] SP proto 1*(SP fmt) CRLF
    const result = /^m=([^ ]*) ([^ ]*)(\/[^ ]*)? ([^ ]*)( .*)+\r\n/.exec(str);
    if (!result) {
        return makeParseError(`cannot parse media: ${str}`);
    }
    const port = parseInt(result[2]);
    if (Number.isNaN(port)) {
        return makeParseError(`invalid port in media: ${result[0]}`);
    }
    const formats = result[5].substring(1).split(' ').map((s) => parseInt(s));
    if (Number.isNaN(formats.find(Number.isNaN))) {
        return makeParseError(`invalid formats: ${result[5].substring(2)}`);
    }    
    return makeParseSuccess({
        media: result[1],
        port,
        ...(result[3] !== undefined ? {integer: result[3]} : {}),
        proto: result[4],
        formats
    }, str.substring(result[0].length));    
}

function parseConnections(str: string): ParseResult<MediaConnection> {
    let conns: ConnectionType[] = [];
    while (str.startsWith('c=')) {
        const r = parseConnection(str);
        if (r.type === 'error') {
            return r;
        }
        conns.push(r.obj.connection!);
        str = r.rest;
    }
    return makeParseSuccess({connection: conns}, str);
}

function parsersReducer<T>(parsers: ((str: string) => ParseResult<any>)[], str: string): ParseResult<T> {
    const result = parsers.reduce((prev: ParseResult<object>, parser): ParseResult<object> => {
        if (prev.type !== 'success') {
            return prev;
        }
        const r = parser(prev.rest);
        if (r.type !== 'success') {
            return r;
        }
        return {
            type: 'success',
            obj: {...prev.obj, ...r.obj},
            rest: r.rest
        };
    }, {type: 'success', obj: {}, rest: str});
    return result as ParseResult<T>;    
}

function makeFullFieldParser<T>(field: string, name: string, creator: (s:string) => T): ((s: string) => ParseResult<T>) {
    const re = new RegExp(`^${field}=(.*)\r\n`);
    return function(str: string): ParseResult<T> {
        const match = str.match(re);
        if (!match) {
            return makeParseError(`cannot parse ${name}: ${str}`);
        }
        return makeParseSuccess(creator(match[1]), str.substring(match[0].length));
    };
}

function makeFullOptFieldParser<T>(field: string, creator: (s:string) => T): ((s: string) => ParseResult<T | {}>) {
    const re = new RegExp(`^${field}=(.*)\r\n`);
    return function(str: string): ParseResult<T | {}> {
        const match = str.match(re);
        if (!match) {
            return makeParseSuccess({}, str);
        }
        return makeParseSuccess(creator(match[1]), str.substring(match[0].length));
    };
}

function makeFullMultFieldParser<T>(field: string, creator: (s: string[]) => T): ((s: string) => ParseResult<T>) {
    const f = makeFullOptFieldParser(field, (s) => s);
    return function(str: string): ParseResult<T> {
        const result: string[] = [];
        while (true) {
            const r = f(str);
            if (r.type === 'success' && typeof r.obj === 'string') {
                result.push(r.obj);
            } else if (r.type === 'success') {
                return makeParseSuccess(creator(result), r.rest);
            } else {
                return r;
            }
            if (str.length === r.rest.length) {
                return makeParseError('Infinite loop');
            }
            str = r.rest;
        }
    };
}
