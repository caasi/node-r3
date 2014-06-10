var r3 = require("./build/Release/r3.node");

function f() {
  var n = new r3.R3(10);
  console.log(n);
  n = null;
};

f();
gc();

