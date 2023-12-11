{
    "targets": [
        {
            "target_name": "echo_backend",
            "sources": [
                "src/echo_backend.cpp"
            ],
            "include_dirs": [
                "../../../src",
                "../../.."
            ],
            "libraries": [
                "-L../../../../_build/src",
                "-lfreewebrtc",
                "-L../../../../_build/node/napi_wrapper",
                "-L../../../../_build/node/node_stun",
                "-L../../../../_build/node/node_ice",
                "-L../../../../_build/node/openssl",
                "-lfreewebrtc_napi_wrapper",
                "-lfreewebrtc_node_stun",
                "-lfreewebrtc_node_ice",
                "-lfreewebrtc_node_openssl"
            ],
            "cflags_cc": [ "-std=c++20" ],
            "xcode_settings": {
                "OTHER_CFLAGS": ["-std=c++20"],
            }
        }
    ]
}
