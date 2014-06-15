var r3 = require("./build/Release/r3.node");
var j;

console.log(r3);

function f() {
  var path, data, n, i;
  var count = 256

  path = "/o";
  data = "o";

  n = new r3.Tree(10);
  console.log(n);

  e = new r3.MatchEntry('/foo');
  console.log(e);

  e.requestMethod = r3.METHOD_GET | r3.METHOD_POST;
  e.path = e.path + "/bar";
  e.host = "localhost";
  e.remoteAddress = "192.168.0.1";
  console.log(e);

  for (i = 0; i < count; ++i) {
    n.insert(path, data);
    path += "/o";
    data += "o";
  }

  n.compile();

  path = "/o";
  for (i = 0; i < count; ++i) {
    if (!n.match("/o")) throw new Error("path not found");
    path += "/o";
  }
};

for (j = 0; j < 1; ++j) {
  f();
}

gc();
