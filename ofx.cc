#include <map>
#include <fstream>
#include <nan.h>
#include <libofx/libofx.h>

int statementCallback(const struct OfxStatementData ofxStatement, void *data)
{
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Object> node = v8::Object::New(isolate);

  struct OfxAccountData ofxAccount = *(ofxStatement.account_ptr);

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

  // Statement specific fields
  if (ofxStatement.ledger_balance_valid) {
    node->Set(Nan::New("balance").ToLocalChecked(),
              Nan::New(ofxStatement.ledger_balance));
  }
  if (ofxStatement.ledger_balance_date_valid) {
    node->Set(Nan::New("balanceDate").ToLocalChecked(),
              v8::Date::New(isolate, ofxStatement.ledger_balance_date * 1000));
  }

  v8::Local<v8::Object> root = *((v8::Local<v8::Object> *) data);
  root->Set(Nan::New("account").ToLocalChecked(), node);
  return 0;
}

int transactionCallback(const struct OfxTransactionData data, void *cbData)
{
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Object> node = v8::Object::New(isolate);

  if (data.amount_valid) {
    node->Set(Nan::New("amount").ToLocalChecked(), Nan::New(data.amount));
  }

  if (data.name_valid) {
    node->Set(Nan::New("name").ToLocalChecked(), Nan::New(data.name).ToLocalChecked());
  }

  if (data.memo_valid) {
    node->Set(Nan::New("memo").ToLocalChecked(), Nan::New(data.memo).ToLocalChecked());
  }

  if (data.transactiontype_valid) {
    node->Set(Nan::New("type").ToLocalChecked(), Nan::New(data.transactiontype));
  }

  if (data.fi_id_valid) {
    node->Set(Nan::New("fitid").ToLocalChecked(), Nan::New(data.fi_id).ToLocalChecked());
  }

  if (data.date_posted_valid) {
    node->Set(Nan::New("datePosted").ToLocalChecked(),
              v8::Date::New(isolate, data.date_posted * 1000));
  }

  // Append the above node to the transactions array
  v8::Local<v8::Array> arrTransactions =
    v8::Local<v8::Array>::Cast(*((v8::Local<v8::Array> *) cbData));
  arrTransactions->Set(arrTransactions->Length(), node);
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
  v8::Local<v8::Array> transactions = v8::Array::New(isolate);
  result->Set(Nan::New("transactions").ToLocalChecked(), transactions);

  // Parse file
  LibofxContextPtr context = libofx_get_new_context();
  ofx_set_statement_cb(context, statementCallback, &result);
  ofx_set_transaction_cb(context, transactionCallback, &transactions);
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

  // TransactionType
  v8::Local<v8::Object> ttNode = v8::Object::New(isolate);
  ttNode->Set(Nan::New("CREDIT").ToLocalChecked(), Nan::New(OFX_CREDIT));
  ttNode->Set(Nan::New("DEBIT").ToLocalChecked(), Nan::New(OFX_DEBIT));
  ttNode->Set(Nan::New("INT").ToLocalChecked(), Nan::New(OFX_INT));
  ttNode->Set(Nan::New("DIV").ToLocalChecked(), Nan::New(OFX_DIV));
  ttNode->Set(Nan::New("FEE").ToLocalChecked(), Nan::New(OFX_FEE));
  ttNode->Set(Nan::New("SRVCHG").ToLocalChecked(), Nan::New(OFX_SRVCHG));
  ttNode->Set(Nan::New("DEP").ToLocalChecked(), Nan::New(OFX_DEP));
  ttNode->Set(Nan::New("ATM").ToLocalChecked(), Nan::New(OFX_ATM));
  ttNode->Set(Nan::New("POS").ToLocalChecked(), Nan::New(OFX_POS));
  ttNode->Set(Nan::New("XFER").ToLocalChecked(), Nan::New(OFX_XFER));
  ttNode->Set(Nan::New("CHECK").ToLocalChecked(), Nan::New(OFX_CHECK));
  ttNode->Set(Nan::New("PAYMENT").ToLocalChecked(), Nan::New(OFX_PAYMENT));
  ttNode->Set(Nan::New("CASH").ToLocalChecked(), Nan::New(OFX_CASH));
  ttNode->Set(Nan::New("DIRECTDEP").ToLocalChecked(), Nan::New(OFX_DIRECTDEP));
  ttNode->Set(Nan::New("DIRECTDEBIT").ToLocalChecked(), Nan::New(OFX_DIRECTDEBIT));
  ttNode->Set(Nan::New("REPEATPMT").ToLocalChecked(), Nan::New(OFX_REPEATPMT));
  ttNode->Set(Nan::New("OTHER").ToLocalChecked(), Nan::New(OFX_OTHER));
  exports->Set(Nan::New("TransactionType").ToLocalChecked(), ttNode);
}

void Init(v8::Local<v8::Object> exports)
{
  Constants(exports);
  exports->Set(Nan::New("parseFile").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(parseFile)->GetFunction());
}

NODE_MODULE(ofx, Init)
