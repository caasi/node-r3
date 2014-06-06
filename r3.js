var r3 = require("./build/Release/r3.node");

function f() {
  new r3.R3(10);
};

f();

gc();
gc();
gc();

