var r3 = require("./build/Release/r3.node");
var j;

function f() {
  var path, data, n, i;
  var count = 256

  path = "/o";
  data = "o";

  n = new r3.Tree(10);

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

for (j = 0; j < 128; ++j) {
  f();
}

gc();
