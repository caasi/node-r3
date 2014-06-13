#include <node.h>
#include <v8.h>
#include <nan.h>
#include <iostream>

namespace r3 {
    #include <r3/r3.h>
}

using namespace v8;

/*
r3::node *tree_find_data(r3::node *n, Local<Value> &data) {
    unsigned int i;
    r3::node *result = NULL;

    if (!n) return NULL;

    Local<Value> current(reinterpret_cast<Value *>(n->data));
    std::cout << (void *)*data << std::endl;
    std::cout << (void *)*current << std::endl;
    if (current == data) return n;

    for (i = 0; i < n->edge_len; ++i) {
        result = tree_find_data(n->edges[i]->child, data);
        if (result) return result;
    }

    return NULL;
}
*/

void tree_dispose_data(r3::node *n) {
    unsigned int i;

    if (!n) return;

    r3::r3_tree_dump(n, 0);

    // TODO: find out if leaking
    Persistent<Value> data(reinterpret_cast<Value *>(n->data));
    if (n) data.Dispose();

    for (i = 0; i < n->edge_len; ++i) {
        tree_dispose_data(n->edges[i]->child);
    }
}

NAN_WEAK_CALLBACK(cleanUp) {
    r3::node *n = static_cast<r3::node *>(data.GetParameter());
    tree_dispose_data(n);
    r3::r3_tree_free(n);
    std::cout << "r3_tree_free();" << std::endl;
}

/*
NAN_WEAK_CALLBACK(keepPayloadDataAlive) {
    r3::node *n = static_cast<r3::node *>(data.GetParameter());
    Local<Value> payload = NanNew(data.GetCallbackInfo()->persistent);

    if (tree_find_data(n, payload)) {
        std::cout << "payload will be alive" << std::endl;
        data.Revive();
    } else {
        std::cout << "payload will be gone" << std::endl;
    }
}
*/
r3::node *get_node(Local<Object> &self) {
    Local<External> external = Local<External>::Cast(self->GetInternalField(0));
    return static_cast<r3::node *>(external->Value());
}

NAN_METHOD(treeInsertPath) {
    NanScope();

    Local<Object> self = args.Holder();

    const String::Utf8Value path(args[0]);

    Persistent<Value> data;
    NanAssignPersistent(data, args[1]);
    std::cout << "raw data ptr: " << (void *)*data << std::endl;

    r3::r3_tree_insert_pathl(get_node(self), *path, path.length(), *data);
    std::cout << "r3_tree_insert_path(n, \"" << *path << "\");" << std::endl;

    NanReturnValue(self);
}

NAN_METHOD(treeCompile) {
    NanScope();

    Local<Object> self = args.Holder();

    char *errstr = NULL;
    int err = r3::r3_tree_compile(get_node(self), &errstr);
    std::cout << "r3_tree_compile();" << std::endl;

    NanReturnValue(self);
}

NAN_METHOD(treeMatch) {
    NanScope();

    Local<Object> self = args.Holder();

    const String::Utf8Value path(args[0]);

    r3::node *matched = r3::r3_tree_matchl(get_node(self), *path, path.length(), NULL);
    std::cout << "r3_tree_match(\"" << *path << "\");" << std::endl;

    if (matched) {
        Local<Value> data(reinterpret_cast<Value *>(matched->data));
        NanReturnValue(data);
    } else {
        NanReturnNull();
    }
}

NAN_METHOD(constructor) {
    if (!args.IsConstructCall()) {
        NanThrowError("Cannot call constructor as function");
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
    instance->Set(NanNew<String>("treeMatch"),
                  NanNew<FunctionTemplate>(treeMatch)->GetFunction());

    NanMakeWeakPersistent(instance, n, &cleanUp);

    NanReturnValue(instance);
}

void init(Handle<Object> exports) {
    exports->Set(NanNew<String>("R3"),
                 NanNew<FunctionTemplate>(constructor)->GetFunction());
}

NODE_MODULE(r3, init);
