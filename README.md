# node-r3

node-r3 is a native binding of [c9s/r3][] routing library, please check it for more information.

## Installation

```bash
$ npm install node-libr3
```

## Testing

```bash
$ npm test
```

## Benchmarking

```bash
$ npm run benchmark
```

## Examples

### API

```JavaScript
r3 = require("node-libr3");

n = new r3.Tree(10);
route_data = 3;

n.insert("/zoo", route_data);
n.insert("/foo/bar", route_data);
n.insert("/post/{id}", route_data);
n.insert("/user/{id:\\d+}", route_data);
n.insert("/foo/bar", route_data);

n.compile();

//should be 3
console.log(n.match("/foo/bar"));
```

### Routing with conditions

```JavaScript
r3 = require("node-libr3");

n = new r3.Tree(10);
route_data = 3;

n.insertRoute(r3.METHOD_GET | r3.METHOD_POST, "/blog/post", route_data);
n.compile();

e = new r3.MatchRoute("/foo/bar");
//e.requestMethod is r3.METHOD_GET by default
e.requestMethod = r3.METHOD_GET;

//should be 3
console.log(n.matchRoute(e));
```

## Alternative

There is another [othree/node-r3][] project use different approach to let node can use r3's feature.

## License

This software is released under MIT License.
