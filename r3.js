var r3 = require("./build/Release/r3.node");

function f() {
  var route_data, n;

  route_data= 3;

  n = new r3.R3(10);
  console.log(n);

  n.treeInsertPath("/bar", function () {
    console.log("foobar");
  });
  n.treeInsertPath("/zoo", route_data);
  n.treeInsertPath("/foo/bar", route_data);
  n.treeInsertPath("/post/{id}", route_data);
  n.treeInsertPath("/user/{id:\\d+}", route_data);
  n.treeCompile();

  n.treeMatch("/bar")();
  console.log(n.treeMatch("/zoo"));
  console.log(n.treeMatch("/2oo"));
};

f();
gc();

