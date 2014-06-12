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
    r3::node *n = static_cast<r3::node *>(data.GetParameter());
    r3::r3_tree_free(n);
    std::cout << "r3_tree_free(n);" << std::endl;
}

NAN_METHOD(treeInsertPath) {
    NanScope();

    Local<Object> self = args.Holder();

    Local<External> external = Local<External>::Cast(self->GetInternalField(0));
    r3::node *n = static_cast<r3::node *>(external->Value());

    const String::Utf8Value path(args[0]);

    // what will happen after i free the tree?
    Persistent<Value> data;
    NanAssignPersistent(data, args[1]);

    r3::r3_tree_insert_pathl(n, *path, path.length(), &data);
    std::cout << "r3_tree_insert_path(n, \"" << *path << "\");" << std::endl;

    NanReturnValue(self);
}

NAN_METHOD(treeCompile) {
    NanScope();

    Local<Object> self = args.Holder();

    Local<External> external = Local<External>::Cast(self->GetInternalField(0));
    r3::node *n = static_cast<r3::node *>(external->Value());
    char *errstr = NULL;
    int err = r3::r3_tree_compile(n, &errstr);
    std::cout << "r3_tree_compile();" << std::endl;

    NanReturnValue(self);
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
    instance->SetInternalField(0, NanNew<External>(n));
    instance->Set(NanNew<String>("treeInsertPath"),
                  NanNew<FunctionTemplate>(treeInsertPath)->GetFunction());
    instance->Set(NanNew<String>("treeCompile"),
                  NanNew<FunctionTemplate>(treeCompile)->GetFunction());

    NanMakeWeakPersistent(instance, n, &cleanUp);

    NanReturnValue(instance);
}

void init(Handle<Object> exports) {
    exports->Set(NanNew<String>("R3"),
                 NanNew<FunctionTemplate>(constructor)->GetFunction());
}

NODE_MODULE(r3, init);
