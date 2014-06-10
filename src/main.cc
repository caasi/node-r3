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
    std::cout << "r3_tree_free() " << std::endl;

    r3::node *n = static_cast<r3::node *>(data.GetParameter());
    r3::r3_tree_free(n);
}

NAN_METHOD(constructor) {
    if (!args.IsConstructCall()) {
        return ThrowException(String::New("Cannot call constructor as function"));
    }

    NanScope();

    int capacity = args[0]->Uint32Value();
    r3::node *n = r3::r3_tree_create(capacity);
    std::cout << "r3_tree_create(" << capacity << ");" << std::endl;

    Handle<ObjectTemplate> r3_template = ObjectTemplate::New();
    r3_template->SetInternalFieldCount(1);

    Local<Object> instance = r3_template->NewInstance();
    instance->SetInternalField(0, NanMakeWeakPersistent(instance, n, &cleanUp)->persistent);

    NanReturnValue(instance);
}

void init(Handle<Object> exports) {
    exports->Set(NanNew<String>("R3"),
                 NanNew<FunctionTemplate>(constructor)->GetFunction());
}

NODE_MODULE(r3, init);
