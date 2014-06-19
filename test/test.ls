expect = require 'expect.js'
r3     = require '../build/Release/r3.node'

n = new r3.Tree
e = new r3.MatchEntry '/foo/buz'
data = \foobar

describe 'MatchEntry' ->
  describe '#construtor()' (,) ->
    it 'should return an instance' ->
      expect(e).not.to.be undefined
      expect(e.path).to.be '/foo/buz'
      expect(e.requestMethod).to.be r3.METHOD_GET
      expect(e.host).to.be ''
      expect(e.remoteAddress).to.be ''

describe 'Tree' ->
  describe '#constructor()' (,) ->
    it 'should return an instance' ->
      expect(n).not.to.be undefined
      expect(n.insert).not.to.be undefined
      expect(n.insertRoute).not.to.be undefined
      expect(n.compile).not.to.be undefined
      expect(n.match).not.to.be undefined
  describe '#insert()' (,) ->
    it 'should return itself without any errors' ->
      expect(n.insert '/foo/bar', data).to.be n
  describe '#insertRoute()' (,) ->
    it 'should return itself without any errors' ->
      expect(n.insertRoute r3.METHOD_POST .|. r3.METHOD_DELETE, '/foo/buz', data).to.be n
  describe '#compile()' (,) ->
    it 'should return itself without any errors' ->
      expect(n.compile!).to.be n
  describe '#match()' (,) ->
    it 'should not match /' ->
      expect(n.match '/').to.be null
    it 'should match /foo/bar' ->
      expect(n.match '/foo/bar').to.be data
    it 'should match MatchEntry even w/ wrong request method' ->
      expect(n.match e).to.be data
    it 'should match MatchEntry w/ correct method' ->
      e.requestMethod = r3.METHOD_POST .|. r3.METHOD_DELETE
      expect(n.match e).to.be data
  describe '#matchRoute()' (,) ->
    it 'should not match MatchEntry w/ wrong request method' ->
      e.requestMethod = r3.METHOD_GET
      expect(n.matchRoute e).to.be null
    it 'should match MatchEntry w/ correct method' ->
      e.requestMethod = r3.METHOD_POST .|. r3.METHOD_DELETE
      expect(n.matchRoute e).to.be data

