var r3 = require("./build/Release/r3.node");

function f() {
  r3.createTree(10);
};

f();

gc();
gc();
gc();

