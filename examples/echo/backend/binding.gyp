{
    "targets": [
        {
            "target_name": "echo_backend",
            "sources": [ "src/echo_backend.cpp" ],
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
