{
    "targets": [
        {
            "target_name": "echo_backend",
            "sources": [
                "src/echo_backend.cpp",
                "src/napi_error.cpp",
                "src/napi_wrapper.cpp",
                "src/napi_stun_message.cpp",
                "src/napi_stun_header.cpp"
            ],
            "include_dirs": [
                "../../../src"
            ],
            "libraries": [
                "-L../../../../_build/src",
                "-lfreewebrtc"
            ],
            "cflags_cc": [ "-std=c++20" ],
            "xcode_settings": {
                "OTHER_CFLAGS": ["-std=c++20"],
            }
        }
    ]
}
