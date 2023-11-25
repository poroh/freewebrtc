# Free WebRTC project

Welcome! Goal of this project is to rethink approach to implementation
of communication protocols in C++. Many other implementations of WebRTC
stack exist in this world but this implementation is different:

- It does not have any strict dependencies (for example openssl is only one option that can be used for crypto functions)
- It is written in software architecture-agnostic way:
  - If you need just STUN message parser it is standalone
  - If you need STUN client implementation - you can get it and put it in your environment by implenting sleeps / sends etc
  - If you want secure random for STUN transaction identifiers you can replace it
  - etc.
- Code highly relies on strong typing
- Thorough unit testing (117 tests and counting)
- No quirky practicies with raw pointers and type casts.
- Test coverage
- Address sanitizer
- -Wall -Werror -pedantic from the beginning
- No global variables and side effects
- References to standards everywhere where these standards implemented

# Maturity

Project is on very early stages in terms of completeness of WebRTC implementation. But pieces that are already built
should be very high level of quality.

What is implemented:
- RFC 2104: HMAC: Keyed-Hashing for Message Authentication
- RFC 8489: Session Traversal Utilities for NAT (STUN)
  - Conformant STUN message parser
  - FINGERPRINT mechanism
  - Authentication with short-term credential mechanism
  - ALTERNATE-SERVER mechanism
  - Backward compatibility with RFC3489
  - All STUN attributes
  - STUN statless server
  - STUN UDP client with full transaction support
  - Initial RTO adjustment using Karn's algorithm and RFC6298
- RFC 5769: Test Vectors for Session Traversal Utilities for NAT (STUN)
- RFC 8445: Interactive Connectivity Establishment (ICE)
  - STUN Extensions (PRIORITY, ICE-CONTROLLED, ICE-CONTROLLING, USE-CANDIDATE)

# Build

Known environment that can compile this code:
- Ubuntu 22.04 with gcc/g++ compiler
- macOS 12.4 with clang/clang++ compiler

To build static library and tests:
```
cd freewebrtc
cmake -S . -B _build
make -C _build
```

Run tests
```
_build/tests/freewebrtc_tests
```

# Node.js examples

## STUN UDP client

And example of STUN client using node.js.

Prerequisite: build freewebrtc (see above)

```
cd freewebrtc/examples/stun_client
npm run build
node ./index.js
```

## Echo server

As an example and to make development closer to real-life node.js example is implemented. 
This is very early stage example of echo server that in future will

- Negotiate ICE connectivity
- Establish DTLS 
- Receive RTP streams and return it back to client

Build and run of the example:

Client:
```
cd freewebrtc/examples/echo/frontend
npm install
npm run start
```

Server:
```
cd freewebrtc/examples/echo/backend
npm install
npm run build
```

After running server you can connect to localhost:8000 and this will be test.
And don't expect too much from it. It is in the early stage.











