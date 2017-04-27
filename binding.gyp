{
  "targets": [
    {
      "target_name": "ofx",
      "sources": [ "ofx.cc" ],
      "ldflags": [ "-lofx" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
