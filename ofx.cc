#include <map>
#include <fstream>
#include <nan.h>
#include <libofx/libofx.h>

#define NEW_STR_LOCAL(x) Nan::New(x).ToLocalChecked()


int statementCallback(const struct OfxStatementData ofxStatement, void *data)
{
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Object> node = v8::Object::New(isolate);

  struct OfxAccountData ofxAccount = *(ofxStatement.account_ptr);

  if (ofxAccount.account_id_valid) {
    node->Set(NEW_STR_LOCAL("id"), NEW_STR_LOCAL(ofxAccount.account_id));
    node->Set(NEW_STR_LOCAL("name"), NEW_STR_LOCAL(ofxAccount.account_name));
  }

  if (ofxAccount.account_number_valid) {
    node->Set(NEW_STR_LOCAL("number"), NEW_STR_LOCAL(ofxAccount.account_number));
  }

  if (ofxAccount.currency_valid) {
    node->Set(NEW_STR_LOCAL("currency"), NEW_STR_LOCAL(ofxAccount.currency));
  }

  if (ofxAccount.account_type_valid) {
    node->Set(NEW_STR_LOCAL("type"), Nan::New(ofxAccount.account_type));
  }

  if (ofxAccount.bank_id_valid) {
    node->Set(NEW_STR_LOCAL("bankId"), NEW_STR_LOCAL(ofxAccount.bank_id));
  }

  if (ofxAccount.branch_id_valid) {
    node->Set(NEW_STR_LOCAL("branchId"), NEW_STR_LOCAL(ofxAccount.branch_id));
  }

  // Statement specific fields
  if (ofxStatement.ledger_balance_valid) {
    node->Set(NEW_STR_LOCAL("balance"), Nan::New(ofxStatement.ledger_balance));
  }
  if (ofxStatement.ledger_balance_date_valid) {
    node->Set(NEW_STR_LOCAL("balanceDate"),
              v8::Date::New(isolate, ofxStatement.ledger_balance_date * 1000));
  }

  v8::Local<v8::Object> root = *((v8::Local<v8::Object> *) data);
  root->Set(NEW_STR_LOCAL("account"), node);
  return 0;
}

int transactionCallback(const struct OfxTransactionData data, void *cbData)
{
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Object> node = v8::Object::New(isolate);

  if (data.amount_valid) {
    node->Set(NEW_STR_LOCAL("amount"), Nan::New(data.amount));
  }

  if (data.name_valid) {
    node->Set(NEW_STR_LOCAL("name"), NEW_STR_LOCAL(data.name));
  }

  if (data.memo_valid) {
    node->Set(NEW_STR_LOCAL("memo"), NEW_STR_LOCAL(data.memo));
  }

  if (data.transactiontype_valid) {
    node->Set(NEW_STR_LOCAL("type"), Nan::New(data.transactiontype));
  }

  if (data.fi_id_valid) {
    node->Set(NEW_STR_LOCAL("fitId"), NEW_STR_LOCAL(data.fi_id));
  }

  if (data.date_posted_valid) {
    node->Set(NEW_STR_LOCAL("datePosted"),
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
  result->Set(NEW_STR_LOCAL("transactions"), transactions);

  // Parse file
  LibofxContextPtr context = libofx_get_new_context();
  ofx_set_statement_cb(context, statementCallback, &result);
  ofx_set_transaction_cb(context, transactionCallback, &transactions);
  libofx_proc_file(context, filePath, OFX);
  libofx_free_context(context);

  args.GetReturnValue().Set(result);
}

void Constants(v8::Local<v8::Object> exports) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();

  // OfxAccountData::AccountType
  v8::Local<v8::Object> atNode = v8::Object::New(isolate);
  atNode->Set(NEW_STR_LOCAL("CMA"), Nan::New(OfxAccountData::OFX_CMA));
  atNode->Set(NEW_STR_LOCAL("SAVINGS"), Nan::New(OfxAccountData::OFX_SAVINGS));
  atNode->Set(NEW_STR_LOCAL("CHECKING"), Nan::New(OfxAccountData::OFX_CHECKING));
  atNode->Set(NEW_STR_LOCAL("MONEYMRKT"), Nan::New(OfxAccountData::OFX_MONEYMRKT));
  atNode->Set(NEW_STR_LOCAL("CREDITLINE"), Nan::New(OfxAccountData::OFX_CREDITLINE));
  atNode->Set(NEW_STR_LOCAL("CREDITCARD"), Nan::New(OfxAccountData::OFX_CREDITCARD));
  atNode->Set(NEW_STR_LOCAL("INVESTMENT"), Nan::New(OfxAccountData::OFX_INVESTMENT));
  exports->Set(NEW_STR_LOCAL("AccountType"), atNode);

  // TransactionType
  v8::Local<v8::Object> ttNode = v8::Object::New(isolate);
  ttNode->Set(NEW_STR_LOCAL("CREDIT"), Nan::New(OFX_CREDIT));
  ttNode->Set(NEW_STR_LOCAL("DEBIT"), Nan::New(OFX_DEBIT));
  ttNode->Set(NEW_STR_LOCAL("INT"), Nan::New(OFX_INT));
  ttNode->Set(NEW_STR_LOCAL("DIV"), Nan::New(OFX_DIV));
  ttNode->Set(NEW_STR_LOCAL("FEE"), Nan::New(OFX_FEE));
  ttNode->Set(NEW_STR_LOCAL("SRVCHG"), Nan::New(OFX_SRVCHG));
  ttNode->Set(NEW_STR_LOCAL("DEP"), Nan::New(OFX_DEP));
  ttNode->Set(NEW_STR_LOCAL("ATM"), Nan::New(OFX_ATM));
  ttNode->Set(NEW_STR_LOCAL("POS"), Nan::New(OFX_POS));
  ttNode->Set(NEW_STR_LOCAL("XFER"), Nan::New(OFX_XFER));
  ttNode->Set(NEW_STR_LOCAL("CHECK"), Nan::New(OFX_CHECK));
  ttNode->Set(NEW_STR_LOCAL("PAYMENT"), Nan::New(OFX_PAYMENT));
  ttNode->Set(NEW_STR_LOCAL("CASH"), Nan::New(OFX_CASH));
  ttNode->Set(NEW_STR_LOCAL("DIRECTDEP"), Nan::New(OFX_DIRECTDEP));
  ttNode->Set(NEW_STR_LOCAL("DIRECTDEBIT"), Nan::New(OFX_DIRECTDEBIT));
  ttNode->Set(NEW_STR_LOCAL("REPEATPMT"), Nan::New(OFX_REPEATPMT));
  ttNode->Set(NEW_STR_LOCAL("OTHER"), Nan::New(OFX_OTHER));
  exports->Set(NEW_STR_LOCAL("TransactionType"), ttNode);
}

void Init(v8::Local<v8::Object> exports)
{
  Constants(exports);
  exports->Set(NEW_STR_LOCAL("parseFile"),
               Nan::New<v8::FunctionTemplate>(parseFile)->GetFunction());
}

NODE_MODULE(ofx, Init)
