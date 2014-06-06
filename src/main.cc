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

static Handle<FunctionTemplate> r3_template;
static Handle<ObjectTemplate> r3_instance_template;

void cleanUp(Persistent<Value> object, void *parameter) {
    r3::node *n = static_cast<r3::node *>(parameter);

    r3::r3_tree_free(n);
    std::cout << "r3_tree_free(): " << n << std::endl;

    object.Dispose();
    object.Clear();
}

NAN_METHOD(constructor) {
    if (!args.IsConstructCall()) {
        return ThrowException(String::New("Cannot call constructor as function"));
    }

    NanScope();

    int capacity = args[0]->Uint32Value();
    r3::node *n = r3::r3_tree_create(capacity);
    std::cout << "r3_tree_create(" << capacity << ");" << std::endl;

    Persistent<Object> instance = Persistent<Object>::New(r3_instance_template->NewInstance());
    instance->SetInternalField(0, External::Wrap(n));
    instance.MakeWeak(n, cleanUp);

    NanReturnValue(instance);

    //int *parameter = new int(0);
    //NanReturnValue(NanMakeWeakPersistent(External::Wrap(n), parameter, &cleanUp)->persistent);
}

void init(Handle<Object> exports) {
    r3_template = FunctionTemplate::New();
    r3_instance_template = r3_template->InstanceTemplate();
    r3_instance_template->SetInternalFieldCount(1);

    exports->Set(NanNew<String>("R3"),
                 NanNew<FunctionTemplate>(constructor)->GetFunction());
}

NODE_MODULE(r3, init);
