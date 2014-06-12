var r3 = require("./build/Release/r3.node");

function f() {
  var n = new r3.R3(10),
      route_data = 3;

  console.log(n);

  n.treeInsertPath("/bar", route_data);
  n.treeInsertPath("/zoo", route_data);
  n.treeInsertPath("/foo/bar", route_data);
  n.treeInsertPath("/post/{id}", route_data);
  n.treeInsertPath("/user/{id:\\d+}", route_data);
  n.treeCompile();

  n = null;
};

f();
gc();

