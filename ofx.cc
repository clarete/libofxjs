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

  if (ofxAccount.account_number_valid) {
    node->Set(Nan::New("number").ToLocalChecked(),
              Nan::New(ofxAccount.account_number).ToLocalChecked());
  }

  if (ofxAccount.currency_valid) {
    node->Set(Nan::New("currency").ToLocalChecked(),
              Nan::New(ofxAccount.currency).ToLocalChecked());
  }

  if (ofxAccount.account_type_valid) {
    node->Set(Nan::New("type").ToLocalChecked(),
              Nan::New(ofxAccount.account_type));
  }

  if (ofxAccount.bank_id_valid) {
    node->Set(Nan::New("bankId").ToLocalChecked(),
              Nan::New(ofxAccount.bank_id).ToLocalChecked());
  }

  if (ofxAccount.branch_id_valid) {
    node->Set(Nan::New("branchId").ToLocalChecked(),
              Nan::New(ofxAccount.branch_id).ToLocalChecked());
  }

  v8::Local<v8::Object> root = *((v8::Local<v8::Object> *) data);
  root->Set(Nan::New("account").ToLocalChecked(), node);
  return 0;
}

void parseFile(const Nan::FunctionCallbackInfo<v8::Value>& args)
{
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

void Constants(v8::Local<v8::Object> exports) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();

  // OfxAccountData::AccountType
  v8::Local<v8::Object> atNode = v8::Object::New(isolate);
  atNode->Set(Nan::New("SAVINGS").ToLocalChecked(), Nan::New(OfxAccountData::OFX_SAVINGS));
  atNode->Set(Nan::New("CHECKING").ToLocalChecked(), Nan::New(OfxAccountData::OFX_CHECKING));
  atNode->Set(Nan::New("MONEYMRKT").ToLocalChecked(), Nan::New(OfxAccountData::OFX_MONEYMRKT));
  atNode->Set(Nan::New("CREDITLINE").ToLocalChecked(), Nan::New(OfxAccountData::OFX_CREDITLINE));
  atNode->Set(Nan::New("CMA").ToLocalChecked(), Nan::New(OfxAccountData::OFX_CMA));
  atNode->Set(Nan::New("CREDITCARD").ToLocalChecked(), Nan::New(OfxAccountData::OFX_CREDITCARD));
  atNode->Set(Nan::New("INVESTMENT").ToLocalChecked(), Nan::New(OfxAccountData::OFX_INVESTMENT));
  exports->Set(Nan::New("AccountType").ToLocalChecked(), atNode);
}

void Init(v8::Local<v8::Object> exports)
{
  Constants(exports);
  exports->Set(Nan::New("parseFile").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(parseFile)->GetFunction());
}

NODE_MODULE(ofx, Init)
