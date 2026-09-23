{
  "targets": [
    {
      "target_name": "gracio_native",
      "sources": [ 
        "src/native/bindings.cpp",
        "../../native/src/gracio_core.cpp",
        "../../native/src/gracio_api.cpp"
      ],
      "include_dirs": [
        "node_modules/node-addon-api",
        "../../native/include"
      ],
      "dependencies": [],
      "defines": [ 
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "msvs_settings": {
        "VCCLCompilerTool": { 
          "ExceptionHandling": 1 
        }
      },
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7"
      },
      "conditions": [
        ["OS=='win'", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          }
        }]
      ]
    }
  ]
}
