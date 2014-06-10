var r3 = require("./build/Release/r3.node");

function f() {
  var n = new r3.R3(10);
  n = null;
};

f();

function doGC() {
  console.log("gc");
  gc();
};

setInterval(doGC, 1000);

