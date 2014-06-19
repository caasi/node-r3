#include <node.h>
#include <v8.h>
#include <nan.h>
#include <cstring>
#include <iostream>

namespace r3 {
    #include <r3/r3.h>
}

using namespace v8;

r3::node        *node_from_v8_obj(const Local<Object> &self);
r3::match_entry *match_entry_from_v8_obj(const Local<Object> &self);
r3::match_entry *valid_match_entry_from_v8_obj(const Local<Object> &self);

/***
 * About r3 node/tree
 */
// operator* and constructors for T* will not work on Node.js 0.11 / V8 3.24
//#define NODE_R3_SAVE_RAW

#ifdef NODE_R3_SAVE_RAW
void *ptr_from_value_raw(const Local<Value> &value) {
    Persistent<Value> data;
    NanAssignPersistent(data, value);
    //std::cout << "raw data ptr: " << *data << std::endl;

    return *value;
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
    if (n->endpoint) NanDisposePersistent(data);
#else
    Persistent<Value> *data = value_from_ptr_persistent(n->data);
    if (n->endpoint) NanDisposePersistent(*data);
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

r3::node *node_from_v8_obj(const Local<Object> &self) {
    Local<External> external = Local<External>::Cast(self->GetInternalField(0));
    return static_cast<r3::node *>(external->Value());
}

NAN_METHOD(treeInsertPath) {
    NanScope();

    Local<Object> self = args.Holder();

    // path will be copied by zstrndup() after instertion
    const String::Utf8Value path(args[0]);

    char *errstr = NULL;
#ifdef NODE_R3_SAVE_RAW
    r3::node *result =
        r3::r3_tree_insert_pathl_ex(
            node_from_v8_obj(self),
            *path, path.length(),
            NULL,
            ptr_from_value_raw(args[1]),
            &errstr
        );
#else
    r3::node *result =
        r3::r3_tree_insert_pathl_ex(
            node_from_v8_obj(self),
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

NAN_METHOD(treeInsertRoute) {
    NanScope();

    Local<Object> self = args.Holder();

    int method = args[0]->IntegerValue();
    const String::Utf8Value path(args[1]);

    char *errstr = NULL;
#ifdef NODE_R3_SAVE_RAW
    r3::route *result =
        r3::r3_tree_insert_routel_ex(
            node_from_v8_obj(self),
            method,
            *path, path.length(),
            ptr_from_value_raw(args[2]),
            &errstr
        );
#else
    r3::route *result =
        r3::r3_tree_insert_routel_ex(
            node_from_v8_obj(self),
            method,
            *path, path.length(),
            ptr_from_value_persistent(args[2]),
            &errstr
        );
#endif
    if (result == NULL) {
        NanThrowError(errstr);
        delete errstr;
    }
    //std::cout << "r3_tree_insert_route(n, " << method << ", \"" << *path << "\");" << std::endl;

    // FIXME: r3_tree_insert_routel_ex() gives me route*,
    // maybe I should not return self.
    NanReturnValue(self);
}

NAN_METHOD(treeCompile) {
    NanScope();

    Local<Object> self = args.Holder();

    char *errstr = NULL;
    int err = r3::r3_tree_compile(node_from_v8_obj(self), &errstr);
    if (err) {
        NanThrowError(errstr);
        delete errstr;
    }
    //std::cout << "r3_tree_compile();" << std::endl;

    NanReturnValue(self);
}

NAN_METHOD(treeMatch) {
    NanScope();

    r3::node *matched = NULL;

    if (args[0]->IsString()) {
        const String::Utf8Value path(args[0]);
        matched = r3::r3_tree_matchl(
            node_from_v8_obj(args.Holder()),
            *path, path.length(),
            NULL
        );
        //std::cout << "r3_tree_match(\"" << *path << "\");" << std::endl;
    } else if (args[0]->IsObject()) {
        Local<Object> obj = args[0].As<Object>();
        matched = r3::r3_tree_match_entry(
            node_from_v8_obj(args.Holder()),
            valid_match_entry_from_v8_obj(obj)
        );
    } else {
        NanThrowError("Argument should be a string or a MatchEntry");
    }

    if (matched) {
#ifdef NODE_R3_SAVE_RAW
        Local<Value> data(reinterpret_cast<Value *>(matched->data));
#else
        Local<Value> data = NanNew(*reinterpret_cast<Persistent<Value> *>(matched->data));
#endif
        // FIXME: what should I return?
        NanReturnValue(data);
    } else {
        NanReturnNull();
    }
}

NAN_METHOD(treeMatchRoute) {
    NanScope();

    r3::route *matched = NULL;

    if (args[0]->IsObject()) {
        Local<Object> obj = args[0].As<Object>();
        matched = r3::r3_tree_match_route(
            node_from_v8_obj(args.Holder()),
            valid_match_entry_from_v8_obj(obj)
        );
    } else {
        NanThrowError("Argument should be a MatchEntry");
    }

    if (matched) {
#ifdef NODE_R3_SAVE_RAW
        Local<Value> data(reinterpret_cast<Value *>(matched->data));
#else
        Local<Value> data = NanNew(*reinterpret_cast<Persistent<Value> *>(matched->data));
#endif
        // FIXME: what should I return?
        NanReturnValue(data);
    } else {
        NanReturnNull();
    }
}

NAN_METHOD(treeConstructor) {
    if (!args.IsConstructCall()) {
        NanThrowError("Cannot call Tree constructor as function");
    }

    NanScope();

    int capacity = args[0]->Uint32Value();
    r3::node *n = r3::r3_tree_create(capacity);
    //std::cout << "r3_tree_create(" << capacity << ");" << std::endl;

    Handle<ObjectTemplate> tree_template = ObjectTemplate::New();
    tree_template->SetInternalFieldCount(1);

    Local<Object> instance = tree_template->NewInstance();
    instance->SetInternalField(0, NanNew<External>(n));
    instance->Set(NanNew<String>("insert"),
                  NanNew<FunctionTemplate>(treeInsertPath)->GetFunction());
    instance->Set(NanNew<String>("insertRoute"),
                  NanNew<FunctionTemplate>(treeInsertRoute)->GetFunction());
    instance->Set(NanNew<String>("compile"),
                  NanNew<FunctionTemplate>(treeCompile)->GetFunction());
    instance->Set(NanNew<String>("match"),
                  NanNew<FunctionTemplate>(treeMatch)->GetFunction());
    instance->Set(NanNew<String>("matchRoute"),
                  NanNew<FunctionTemplate>(treeMatchRoute)->GetFunction());

    NanMakeWeakPersistent(instance, n, &treeCleanUp);

    NanReturnValue(instance);
}

/***
 * About MatchEntry
 */
r3::match_entry *match_entry_from_v8_obj(const Local<Object> &self) {
    Local<External> external = Local<External>::Cast(self->GetInternalField(0));
    return static_cast<r3::match_entry *>(external->Value());
}

r3::match_entry *valid_match_entry_from_v8_obj(const Local<Object> &obj) {
    // TODO: create match_entry if entry is a JSON Object
    if (
        !obj->InternalFieldCount() == 1 ||
        !obj->HasOwnProperty(NanNew<String>("requestMethod")) ||
        !obj->HasOwnProperty(NanNew<String>("path")) ||
        !obj->HasOwnProperty(NanNew<String>("host")) ||
        !obj->HasOwnProperty(NanNew<String>("remoteAddress"))
    ) {
        NanThrowError("Not a MatchEntry");
        return NULL;
    }

    return match_entry_from_v8_obj(obj);
}

NAN_GETTER(entryGetMethod) {
    NanScope();

    r3::match_entry *e = match_entry_from_v8_obj(args.Holder());

    NanReturnValue(NanNew<Integer>(e->request_method));
}

NAN_SETTER(entrySetMethod) {
    NanScope();

    r3::match_entry *e = match_entry_from_v8_obj(args.Holder());
    e->request_method = value->ToInteger()->Value();
}

NAN_GETTER(entryGetString) {
    NanScope();

    r3::match_entry *e = match_entry_from_v8_obj(args.Holder());

    String::Utf8Value p(property);

    if (0 == strncmp("path", *p, p.length())) {
        NanReturnValue(NanNew<String>(e->path, e->path_len));
    }
    if (0 == strncmp("host", *p, p.length())) {
        NanReturnValue(NanNew<String>(e->host, e->host_len));
    }
    if (0 == strncmp("remoteAddress", *p, p.length())) {
        NanReturnValue(NanNew<String>(e->remote_addr, e->remote_addr_len));
    }

    //should not happen
    NanReturnNull();
}

NAN_SETTER(entrySetString) {
    NanScope();

    Local<Object> self = args.Holder();

    r3::match_entry *e = match_entry_from_v8_obj(self);
    String::Utf8Value p(property);
    String::Utf8Value v(value);

    // FIXME: should check if strndup available
    if (0 == strncmp("path", *p, p.length())) {
        if (e->path) delete e->path;
        e->path = strndup(*v, v.length());
        e->path_len = v.length();
    }
    if (0 == strncmp("host", *p, p.length())) {
        if (e->host) delete e->host;
        e->host = strndup(*v, v.length());
        e->host_len = v.length();
    }
    if (0 == strncmp("remoteAddress", *p, p.length())) {
        if (e->remote_addr) delete e->remote_addr;
        e->remote_addr = strndup(*v, v.length());
        e->remote_addr_len = v.length();
    }
}

NAN_WEAK_CALLBACK(entryCleanUp) {
    r3::match_entry *e = static_cast<r3::match_entry *>(data.GetParameter());
    r3::match_entry_free(e);
    //std::cout << "match_entry_free();" << std::endl;
}

NAN_METHOD(matchEntryConstructor) {
    if (!args.IsConstructCall()) {
        NanThrowError("Cannot call MatchEntry constructor as function");
    }

    NanScope();

    // path will not be copied after creation
    const String::Utf8Value path(args[0]);

    r3::match_entry *e = r3::match_entry_createl(strndup(*path, path.length()), path.length());
    //std::cout << "match_entry_create(\"" << *path << "\");" << std::endl;
    // TODO: should remove after match_entry_createl() changed
    e->request_method = METHOD_GET;
    e->host = strndup("", 0);
    e->host_len = 0;
    e->remote_addr = strndup("", 0);
    e->remote_addr_len = 0;

    Handle<ObjectTemplate> entry_template = ObjectTemplate::New();
    entry_template->SetInternalFieldCount(1);
    entry_template->SetAccessor(
        NanNew<String>("requestMethod"),
        entryGetMethod,
        entrySetMethod
    );
    entry_template->SetAccessor(
        NanNew<String>("path"),
        entryGetString,
        entrySetString
    );
    entry_template->SetAccessor(
        NanNew<String>("host"),
        entryGetString,
        entrySetString
    );
    entry_template->SetAccessor(
        NanNew<String>("remoteAddress"),
        entryGetString,
        entrySetString
    );

    Local<Object> instance = entry_template->NewInstance();
    instance->SetInternalField(0, NanNew<External>(e));

    NanMakeWeakPersistent(instance, e, &entryCleanUp);

    NanReturnValue(instance);
}

void init(Handle<Object> exports) {
    exports->Set(NanNew<String>("METHOD_GET"),
                 NanNew<Integer>(METHOD_GET));
    exports->Set(NanNew<String>("METHOD_POST"),
                 NanNew<Integer>(METHOD_POST));
    exports->Set(NanNew<String>("METHOD_PUT"),
                 NanNew<Integer>(METHOD_PUT));
    exports->Set(NanNew<String>("METHOD_DELETE"),
                 NanNew<Integer>(METHOD_DELETE));
    exports->Set(NanNew<String>("METHOD_PATCH"),
                 NanNew<Integer>(METHOD_PATCH));
    exports->Set(NanNew<String>("METHOD_HEAD"),
                 NanNew<Integer>(METHOD_HEAD));
    exports->Set(NanNew<String>("METHOD_OPTIONS"),
                 NanNew<Integer>(METHOD_OPTIONS));
    exports->Set(NanNew<String>("Tree"),
                 NanNew<FunctionTemplate>(treeConstructor)->GetFunction());
    exports->Set(NanNew<String>("MatchEntry"),
                 NanNew<FunctionTemplate>(matchEntryConstructor)->GetFunction());
}

NODE_MODULE(r3, init);
