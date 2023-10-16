//
// Copyright (c) 2023 Dmitry Poroh
// All rights reserved.
// Distributed under the terms of the MIT License. See the LICENSE file.
//
// WebRTC-aware SDP
//

import { MediaDescriptor, SDP } from "./sdp";

export type WebrtcSDP = {
    transports: Bundle[];
};

type Bundle = {
    ice: {
        ufrag?: string;
        pwd?: string;
        options: string[];
        candidates: string[];
    },
    dtls: {
        fingerprint?: string;
        setup?: string;
    }
};

export function webrtcProcessSDP(sdp: SDP) {
    const bmg = bundledMediaGroups(sdp);
    const transports = bmg.map((medias) => {
        const bundles = medias.map(({attrs}) => parseTransportAttrs(attrs));
        if (bundles.length === 0) {
            throw new Error(`Empty bundle`);
        }
        const result = bundles[0];
        bundles.forEach((bundle, index) => {
            if (index === 0) {
                return;
            }
            if (!bundlesAreEqual(bundle, result)) {
                throw new Error(`ICE attributes of medias are not equal: ${JSON.stringify(medias)}`);
            }

        });
        return result;
    });
    return {transports};
}


function bundledMediaGroups(sdp: SDP): MediaDescriptor[][] {
    // Bundle MID groups
    return sdp.attrs
        .filter((attr) => attr.startsWith('group:BUNDLE'))
        .map((attr) => {
            return attr.split(' ').splice(1);
        })
        .map((mids) => {
            return mids.map((mid) => {
                const media = sdp.media.find(({attrs}) => attrs.find(attr => attr === `mid:${mid}`));
                if (media === undefined) {
                    throw new Error(`MID is not found: ${mid}`);
                }
                return media;
            });
        });
}

function parseTransportAttrs(attrs: string[]): Bundle {
    let result: Bundle = {ice: {options:[], candidates: []}, dtls: {}};
    attrs.filter((attr) => attr.startsWith('ice-') || attr.startsWith('fingerprint') || attr.startsWith('setup') || attr.startsWith('candidate'))
        .forEach((attr) => {
            const idx = attr.indexOf(':');
            if (idx === -1) {
                throw new Error(`Invalid transport attribute: ${attr}`);
            }
            const name = attr.substring(0, idx);
            const val = attr.substring(idx+1);
            switch (name) {
                case 'ice-ufrag': result.ice.ufrag = val; break;
                case 'ice-pwd': result.ice.pwd = val; break;
                case 'ice-options': result.ice.options = val.split(' '); break;
                case 'setup': result.dtls.setup = val; break;
                case 'fingerprint': result.dtls.fingerprint = val; break;
                case 'candidate': result.ice.candidates.push(val); break;
            }
        });
    return result;
}

function bundlesAreEqual(a: Bundle, b: Bundle): boolean {
    return a.ice.ufrag === b.ice.ufrag
        && a.ice.pwd   === b.ice.pwd
        && JSON.stringify([...a.ice.options].sort()) === JSON.stringify([...b.ice.options].sort())
        && a.dtls.setup === b.dtls.setup
        && a.dtls.fingerprint === b.dtls.fingerprint;

}
