#include <node.h>
#include <v8.h>
#include <nan.h>
#include <iostream>

namespace r3 {
    extern "C" {
        #include <r3/r3.h>
    };
}

using namespace v8;

NAN_WEAK_CALLBACK(cleanUp) {
    int *parameter = data.GetParameter();
    std::cout << "r3_tree_free(): " << *parameter << std::endl;

    r3::node *n = (r3::node*)External::Unwrap(data.GetValue());
    r3::r3_tree_free(n);

    if ((*parameter)++ == 0) {
        data.Revive();
    } else {
        delete parameter;
    }
}

NAN_METHOD(createTree) {
    NanScope();

    int capacity = args[0]->Uint32Value();
    r3::node* n = r3::r3_tree_create(capacity);
    std::cout << "r3_tree_create(" << capacity << ");" << std::endl;

    int *parameter = new int(0);

    NanReturnValue(NanMakeWeakPersistent(External::Wrap(n), parameter, &cleanUp)->persistent);
}

void init(Handle<Object> exports) {
    exports->Set(NanNew<String>("createTree"),
                 NanNew<FunctionTemplate>(createTree)->GetFunction());
}

NODE_MODULE(r3, init);
