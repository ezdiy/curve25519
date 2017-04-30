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
      "src/point.c",
      "src/edpoint.c",
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
      "test/test-field-sqr.c",
      "test/test-field-combined.c",
      "test/test-field-inv.c",
      "test/test-point-dbl.c",
      "test/test-point-diff-add.c",
      "test/test-point-normalize.c",
      "test/test-ed-point-unpack.c",
      "test/test-ed-point-dbl.c",
    ],
  }],
}
