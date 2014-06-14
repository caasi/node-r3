#include <node.h>
#include <v8.h>
#include <nan.h>
#include <iostream>

namespace r3 {
    #include <r3/r3.h>
}

using namespace v8;

#define NODE_R3_SAVE_RAW

#ifdef NODE_R3_SAVE_RAW
void *ptr_from_value_raw(const Local<Value> &value) {
    Persistent<Value> data;
    NanAssignPersistent(data, value);
    //std::cout << "raw data ptr: " << *data << std::endl;

    return *data;
}
#else
void *ptr_from_value_persistent(const Local<Value> &value) {
    Persistent<Value> *data = new Persistent<Value>();
    NanAssignPersistent(*data, value);
    //std::cout << "persistent ptr: " << data << std::endl;

    return data;
}

Persistent<Value> *value_from_ptr_persistent(void *ptr) {
    return reinterpret_cast<Persistent<Value> *>(ptr);
}
#endif

void tree_dispose_data(r3::node *n) {
    unsigned int i;

    if (!n) return;

    //r3::r3_tree_dump(n, 0);

    // TODO: find out if leaking
#ifdef NODE_R3_SAVE_RAW
    Persistent<Value> data(reinterpret_cast<Value *>(n->data));
    if (n->endpoint) data.Dispose();
#else
    Persistent<Value> *data = value_from_ptr_persistent(n->data);
    if (n->endpoint) data->Dispose();
    delete data;
#endif

    for (i = 0; i < n->edge_len; ++i) {
        tree_dispose_data(n->edges[i]->child);
    }
}

NAN_WEAK_CALLBACK(treeCleanUp) {
    r3::node *n = static_cast<r3::node *>(data.GetParameter());
    tree_dispose_data(n);
    r3::r3_tree_free(n);
    //std::cout << "r3_tree_free();" << std::endl;
}

r3::node *get_node(Local<Object> &self) {
    Local<External> external = Local<External>::Cast(self->GetInternalField(0));
    return static_cast<r3::node *>(external->Value());
}

NAN_METHOD(treeInsertPath) {
    NanScope();

    Local<Object> self = args.Holder();

    const String::Utf8Value path(args[0]);

    char *errstr = NULL;
#ifdef NODE_R3_SAVE_RAW
    r3::node *result =
        r3::r3_tree_insert_pathl_ex(
            get_node(self),
            *path, path.length(),
            NULL,
            ptr_from_value_raw(args[1]),
            &errstr
        );
#else
    r3::node *result =
        r3::r3_tree_insert_pathl_ex(
            get_node(self),
            *path, path.length(),
            NULL,
            ptr_from_value_persistent(args[1]),
            &errstr
        );
#endif
    if (result == NULL) {
        NanThrowError(errstr);
        delete errstr;
    }
    //std::cout << "r3_tree_insert_path(n, \"" << *path << "\");" << std::endl;

    NanReturnValue(self);
}

NAN_METHOD(treeCompile) {
    NanScope();

    Local<Object> self = args.Holder();

    char *errstr = NULL;
    int err = r3::r3_tree_compile(get_node(self), &errstr);
    if (err) {
        NanThrowError(errstr);
        delete errstr;
    }
    //std::cout << "r3_tree_compile();" << std::endl;

    NanReturnValue(self);
}

NAN_METHOD(treeMatch) {
    NanScope();

    Local<Object> self = args.Holder();

    const String::Utf8Value path(args[0]);

    r3::node *matched = r3::r3_tree_matchl(get_node(self), *path, path.length(), NULL);
    //std::cout << "r3_tree_match(\"" << *path << "\");" << std::endl;

    if (matched) {
#ifdef NODE_R3_SAVE_RAW
        Local<Value> data(reinterpret_cast<Value *>(matched->data));
#else
        Local<Value> data = NanNew(*reinterpret_cast<Persistent<Value> *>(matched->data));
#endif
        NanReturnValue(data);
    } else {
        NanReturnNull();
    }
}

NAN_METHOD(treeConstructor) {
    if (!args.IsConstructCall()) {
        NanThrowError("Cannot call constructor as function");
    }

    NanScope();

    int capacity = args[0]->Uint32Value();
    r3::node *n = r3::r3_tree_create(capacity);
    //std::cout << "r3_tree_create(" << capacity << ");" << std::endl;

    Handle<ObjectTemplate> r3_template = ObjectTemplate::New();
    r3_template->SetInternalFieldCount(1);

    Local<Object> instance = r3_template->NewInstance();
    instance->SetInternalField(0, NanNew<External>(n));
    instance->Set(NanNew<String>("insert"),
                  NanNew<FunctionTemplate>(treeInsertPath)->GetFunction());
    instance->Set(NanNew<String>("compile"),
                  NanNew<FunctionTemplate>(treeCompile)->GetFunction());
    instance->Set(NanNew<String>("match"),
                  NanNew<FunctionTemplate>(treeMatch)->GetFunction());

    NanMakeWeakPersistent(instance, n, &treeCleanUp);

    NanReturnValue(instance);
}

void init(Handle<Object> exports) {
    exports->Set(NanNew<String>("Tree"),
                 NanNew<FunctionTemplate>(treeConstructor)->GetFunction());
}

NODE_MODULE(r3, init);
