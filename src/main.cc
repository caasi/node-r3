#include <node.h>
#include <v8.h>

namespace r3 {
    extern "C" {
        #include <r3/r3.h>
    };
}

using namespace v8;

Handle<Value> treeCreate(const Arguments &args) {
    HandleScope scope;

    if (args.Length() < 1) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments.")));
        return scope.Close(Undefined());
    }

    return scope.Close(External::Wrap(r3::r3_tree_create(args[0]->IntegerValue())));
}

Handle<Value> treeFree(const Arguments &args) {
    HandleScope scope;

    if (args.Length() < 1) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments.")));
        return scope.Close(Undefined());
    }

    r3::r3_tree_free((r3::node*)External::Unwrap(args[0]));

    return scope.Close(Undefined());
}

void init(Handle<Object> target) {
    target->Set(String::NewSymbol("treeCreate"),
                FunctionTemplate::New(treeCreate)->GetFunction());
    target->Set(String::NewSymbol("treeFree"),
                FunctionTemplate::New(treeFree)->GetFunction());
}

NODE_MODULE(r3, init);
