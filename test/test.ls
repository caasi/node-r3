r3 = require '../build/Release/r3.node'

console.log r3

f = ->
  count = 256

  path = '/o'
  data = 'o';

  n = new r3.Tree 10
  console.log n

  e = new r3.MatchEntry '/o'
  console.log e

  e.requestMethod = r3.METHOD_GET .|. r3.METHOD_POST
  e.host = 'localhost'
  e.remoteAddress = '192.168.0.1'
  console.log e

  for from 0 til count
    n
      ..insert path, data
      ..insertRoute r3.METHOD_GET .|. r3.METHOD_POST, path, data
    path += '/o'
    data += 'o'

  n.compile!

  path = '/o'
  for from 0 til count
    throw new Error 'path not found' if not n.match "/o"
    throw new Error 'match entry not found' if not n.matchRoute e
    path += '/o'
    e.path += '/o'

f!
gc!

