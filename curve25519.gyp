{
  "variables": {
    "gypkg_deps": [
    ],
    "gypkg_test_deps": [
      "git://github.com/indutny/mini-test.c.git@^1.0.0 => mini-test.gyp:mini-test",
    ],
  },

  "targets": [ {
    "target_name": "curve25519",
    "type": "<!(gypkg type)",

    "dependencies": [
      "<!@(gypkg deps <(gypkg_deps))",
    ],

    "direct_dependent_settings": {
      "include_dirs": [
        "include",
      ],
    },

    "include_dirs": [
      ".",
    ],

    "sources": [
      "src/curve25519.c",
      "src/field.c",
    ],
  }, {
    "target_name": "curve25519-test",
    "type": "executable",

    "dependencies": [
      "curve25519",
      "<!@(gypkg deps <(gypkg_deps))",
      "<!@(gypkg deps <(gypkg_test_deps))",
    ],

    "include_dirs": [
      ".",
    ],

    "sources": [
      "test/main.c",
      "test/test-field-add.c",
      "test/test-field-sub.c",
      "test/test-field-mul.c",
      "test/test-field-combined.c",
    ],
  }],
}
