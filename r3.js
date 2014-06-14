var r3 = require("./build/Release/r3.node");
var j;

function f() {
  var path, data, n, i;

  path = "/o";
  data = "o";

  n = new r3.R3(10);

  for (i = 0; i < 256; ++i) {
    n.treeInsertPath(path, data);
    path += "/o";
    data += "o";
  }

  n.treeCompile();

  path = "/o";
  for (i = 0; i < 256; ++i) {
    if (!n.treeMatch("/o")) throw new Error("path not found");
    path += "/o";
  }
};

for (j = 0; j < 256; ++j) {
  f();
  gc();
  console.log(process.memoryUsage());
}
