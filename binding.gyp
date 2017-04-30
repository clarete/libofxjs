{
  "targets": [
    {
      "target_name": "ofx",
      "sources": [ "ofx.cc" ],
      "cflags": [
        "<!@(pkg-config libofx --cflags)"
      ],
      "libraries": [
        "<!@(pkg-config libofx --libs)"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
