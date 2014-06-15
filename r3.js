var r3 = require("./build/Release/r3.node");
var j;

console.log(r3);

function f() {
  var path, data, n, i;
  var count = 256

  path = "/o";
  data = "o";

  n = new r3.Tree(10);
  e = new r3.MatchEntry('/');

  console.log(e, e.requestMethod);

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
