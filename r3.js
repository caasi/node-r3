var r3 = require("./build/Release/r3.node");
var n = r3.treeCreate(10);

console.log(n);

r3.treeFree(n);

