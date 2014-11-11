{
  "targets": [
    {
      "target_name": "oracleBindings",
      "sources": [
        "src/oracleBindings.cc",
        "src/config.cc",
        "src/oracleObject.cc",
        "src/oci_text.cc",
        "src/oci_interface.cc",
        "src/ocip_interface.cc",
        "src/oracleError.cc",
        "src/nodeUtilities.cc",
        "src/global.cc"
      ],
      "conditions": [
        ["OS=='mac'", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "GCC_ENABLE_CPP_RTTI": "YES"
          }
        }],
        ["OS!='win'", {
          "variables": {
            "oci_include_dir%": "<!(if [ -z $OCI_INCLUDE_DIR ]; then echo \"/opt/instantclient/sdk/include/\"; else echo $OCI_INCLUDE_DIR; fi)",
            "oci_lib_dir%": "<!(if [ -z $OCI_LIB_DIR ]; then echo \"/opt/instantclient/\"; else echo $OCI_LIB_DIR; fi)",
            "use_oracle_dummy%": "<!(if [ -z $USE_ORACLE_DUMMY ]; then echo 0; else echo 1; fi)",
            "oci_library%": "<!(if [ -z $USE_ORACLE_DUMMY ]; then echo \"-lclntsh\"; else echo \"\"; fi)"
          },
          "defines": [
            "USE_LINUX=1",
            "USE_ORACLE_DUMMY=<(use_oracle_dummy)"
          ],
          "link_settings": {"libraries": ["-L<(oci_lib_dir)"]},
          "libraries": [ "<(oci_library)" ]
        }],
        ["OS=='win'", {
          "configurations": {
            "Release": {
              "msvs_settings": {
                "VCCLCompilerTool": {
                  "RuntimeLibrary": "2"
                }
              },
            },
            "Debug": {
              "msvs_settings": {
                "VCCLCompilerTool": {
                  "RuntimeLibrary": "3"
                }
              },
            }
          },
          "variables": {
            "oci_include_dir%": "<!(IF DEFINED OCI_INCLUDE_DIR (echo %OCI_INCLUDE_DIR%) ELSE (echo C:\oracle\instantclient\sdk\include))",
            "oci_lib_dir%": "<!(IF DEFINED OCI_LIB_DIR (echo %OCI_LIB_DIR%) ELSE (echo C:\oracle\instantclient\sdk\lib\msvc))",
            "use_oracle_dummy%": "<!(if DEFINED USE_ORACLE_DUMMY (echo 1) ELSE (echo 0))"
          },
          "defines": [
            "USE_ORACLE_DUMMY=<(use_oracle_dummy)"
          ],
          "cflags": [ "/Wall" ],
          "cflags_cc": [ "/Wall" ],
          "link_settings": {"libraries": [ "<(oci_lib_dir)\oci.lib"] }
        }]
      ],
      "include_dirs": [
        "<(oci_include_dir)",
        "<!(node -e \"require('nan')\")"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
