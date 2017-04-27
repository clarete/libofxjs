#include <map>
#include <fstream>
#include <nan.h>
#include <libofx/libofx.h>

int accountCallback(const struct OfxAccountData ofxAccount, void *data)
{
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Object> node = v8::Object::New(isolate);

  if (ofxAccount.account_id_valid) {
    node->Set(Nan::New("id").ToLocalChecked(),
              Nan::New(ofxAccount.account_id).ToLocalChecked());
    node->Set(Nan::New("name").ToLocalChecked(),
              Nan::New(ofxAccount.account_name).ToLocalChecked());
  }

  if (ofxAccount.currency_valid) {
    node->Set(Nan::New("currency").ToLocalChecked(),
              Nan::New(ofxAccount.currency).ToLocalChecked());
  }

  if (ofxAccount.account_type_valid) {
    std::map<int,std::string> types;
    types[OfxAccountData::OFX_CHECKING] = "checking";
    types[OfxAccountData::OFX_SAVINGS] = "savings";
    types[OfxAccountData::OFX_MONEYMRKT] = "money-market";
    types[OfxAccountData::OFX_CREDITLINE] = "credit-line";
    types[OfxAccountData::OFX_CMA] = "cma";
    types[OfxAccountData::OFX_CREDITCARD] = "credit-card";
    types[OfxAccountData::OFX_INVESTMENT] = "investment";
    node->Set(Nan::New("type").ToLocalChecked(),
              Nan::New(types[ofxAccount.account_type]).ToLocalChecked());
  }

  v8::Local<v8::Object> root = *((v8::Local<v8::Object> *) data);
  root->Set(Nan::New("account").ToLocalChecked(), node);
  return 0;
}

void parseFile(const Nan::FunctionCallbackInfo<v8::Value>& args) {
  Nan::Utf8String filePathArg(args[0]);
  char *filePath = *filePathArg;

  // Some sanity check
  std::ifstream f(filePath);
  if (!f.good()) {
    Nan::ThrowError("File not found");
    return;
  }

  // Create result object
  v8::Isolate* isolate = args.GetIsolate();
  v8::Local<v8::Object> result = v8::Object::New(isolate);

  // Parse file
  LibofxContextPtr context = libofx_get_new_context();
  ofx_set_account_cb(context, accountCallback, &result);
  libofx_proc_file(context, filePath, OFX);

  args.GetReturnValue().Set(result);
}

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("parseFile").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(parseFile)->GetFunction());
}

NODE_MODULE(ofx, Init)
