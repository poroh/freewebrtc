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

# Maturity

Project is on very early stages in terms of completeness of WebRTC implementation. But pieces that are already built
should be very high level of quality.


