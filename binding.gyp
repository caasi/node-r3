{
  "targets": [
    {
      "target_name": "r3",
      "sources": ["./src/main.cc"],
      "link_settings": {
        "ldflags": [
          "-Wl,-rpath,/usr/local/lib",
          "<!@(pkg-config r3 --libs-only-L)"
        ],
        "libraries": [
          "<!@(pkg-config r3 --libs-only-l)"
        ]
      }
    }
  ]
}
