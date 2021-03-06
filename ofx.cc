// Copyright (C) 2017-2019 Lincoln de Sousa <lincoln@clarete.li>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <map>
#include <fstream>
#include <nan.h>
#include <libofx/libofx.h>

#define NEW_STR_LOCAL(x) Nan::New<v8::String>(x).ToLocalChecked()

typedef std::map< OfxAccountData *, v8::Local<v8::Array> > account_map_t;

typedef struct {
  v8::Local<v8::Array> root;
  account_map_t map;
} account_array_state_t;

static v8::Local<v8::Object>
accountInfo(const struct OfxAccountData* account)
{
  auto isolate = v8::Isolate::GetCurrent();
  auto node = v8::Object::New(isolate);

  if (account->account_id_valid) {
    node->Set(NEW_STR_LOCAL("acctId"), NEW_STR_LOCAL(account->account_id));
    node->Set(NEW_STR_LOCAL("name"), NEW_STR_LOCAL(account->account_name));
  }
  if (account->account_number_valid) {
    node->Set(NEW_STR_LOCAL("number"), NEW_STR_LOCAL(account->account_number));
  }
  if (account->currency_valid) {
    node->Set(NEW_STR_LOCAL("currency"), NEW_STR_LOCAL(account->currency));
  }
  if (account->account_type_valid) {
    node->Set(NEW_STR_LOCAL("type"), Nan::New(account->account_type));
  }
  if (account->bank_id_valid) {
    node->Set(NEW_STR_LOCAL("bankId"), NEW_STR_LOCAL(account->bank_id));
  }
  if (account->branch_id_valid) {
    node->Set(NEW_STR_LOCAL("branchId"), NEW_STR_LOCAL(account->branch_id));
  }

  return node;
}

static v8::Local<v8::Object>
accountBalance(const struct OfxStatementData& statement)
{
  auto isolate = v8::Isolate::GetCurrent();
  auto node = v8::Object::New(isolate);

  if (statement.ledger_balance_valid) {
    node->Set(NEW_STR_LOCAL("ledger"), Nan::New(statement.ledger_balance));
  }
  if (statement.ledger_balance_date_valid) {
    auto date = Nan::New<v8::Date>(statement.ledger_balance_date * 1000).ToLocalChecked();
    node->Set(NEW_STR_LOCAL("ledgerDate"), date);
  }
  return node;
}

static int
statementCallback(const struct OfxStatementData ofxStatement, void *data)
{
  account_array_state_t *state = (account_array_state_t *) data;
  auto isolate = v8::Isolate::GetCurrent();
  auto account = v8::Object::New(isolate);
  auto transactions = v8::Array::New(isolate);
  auto root = v8::Local<v8::Array>::Cast(state->root);
  account->Set(NEW_STR_LOCAL("info"), accountInfo(ofxStatement.account_ptr));
  account->Set(NEW_STR_LOCAL("balance"), accountBalance(ofxStatement));
  account->Set(NEW_STR_LOCAL("transactions"), transactions);
  root->Set(root->Length(), account);
  state->map[ofxStatement.account_ptr] = transactions;
  return 0;
}

static int
transactionCallback(const struct OfxTransactionData data, void *cbData)
{
  account_array_state_t *state = (account_array_state_t *) cbData;
  auto isolate = v8::Isolate::GetCurrent();
  auto node = v8::Object::New(isolate);

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
    auto date = Nan::New<v8::Date>(data.date_posted * 1000).ToLocalChecked();
    node->Set(NEW_STR_LOCAL("datePosted"), date);
  }

  // Append the above node to the transactions array
  v8::Local<v8::Array> transactions = state->map[data.account_ptr];
  transactions->Set(transactions->Length(), node);
  return 0;
}
NAN_METHOD(parseFile)
{
  Nan::Utf8String filePathArg(info[0]);
  char *filePath = *filePathArg;

  // Some sanity check
  std::ifstream f(filePath);
  if (!f.good()) {
    Nan::ThrowError("File not found");
    return;
  }

  // Create result object
  auto isolate = info.GetIsolate();
  auto accounts = v8::Array::New(isolate);

  // Save pointers for account objects
  account_map_t map;
  account_array_state_t state = { accounts, map };

  // Parse file
  LibofxContextPtr context = libofx_get_new_context();
  ofx_set_statement_cb(context, statementCallback, &state);
  ofx_set_transaction_cb(context, transactionCallback, &state);
  libofx_proc_file(context, filePath, OFX);
  libofx_free_context(context);

  info.GetReturnValue().Set(accounts);
}

static void
Constants(v8::Local<v8::Object> exports) {
  auto isolate = v8::Isolate::GetCurrent();

  // OfxAccountData::AccountType
  auto atNode = v8::Object::New(isolate);
  atNode->Set(NEW_STR_LOCAL("CMA"), Nan::New(OfxAccountData::OFX_CMA));
  atNode->Set(NEW_STR_LOCAL("SAVINGS"), Nan::New(OfxAccountData::OFX_SAVINGS));
  atNode->Set(NEW_STR_LOCAL("CHECKING"), Nan::New(OfxAccountData::OFX_CHECKING));
  atNode->Set(NEW_STR_LOCAL("MONEYMRKT"), Nan::New(OfxAccountData::OFX_MONEYMRKT));
  atNode->Set(NEW_STR_LOCAL("CREDITLINE"), Nan::New(OfxAccountData::OFX_CREDITLINE));
  atNode->Set(NEW_STR_LOCAL("CREDITCARD"), Nan::New(OfxAccountData::OFX_CREDITCARD));
  atNode->Set(NEW_STR_LOCAL("INVESTMENT"), Nan::New(OfxAccountData::OFX_INVESTMENT));
  exports->Set(NEW_STR_LOCAL("AccountType"), atNode);

  // TransactionType
  auto ttNode = v8::Object::New(isolate);
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

  // TransactionTypeNames
  auto ttNameNode = v8::Object::New(isolate);
  ttNameNode->Set(Nan::New(OFX_CREDIT), NEW_STR_LOCAL("CREDIT"));
  ttNameNode->Set(Nan::New(OFX_DEBIT), NEW_STR_LOCAL("DEBIT"));
  ttNameNode->Set(Nan::New(OFX_INT), NEW_STR_LOCAL("INT"));
  ttNameNode->Set(Nan::New(OFX_DIV), NEW_STR_LOCAL("DIV"));
  ttNameNode->Set(Nan::New(OFX_FEE), NEW_STR_LOCAL("FEE"));
  ttNameNode->Set(Nan::New(OFX_SRVCHG), NEW_STR_LOCAL("SRVCHG"));
  ttNameNode->Set(Nan::New(OFX_DEP), NEW_STR_LOCAL("DEP"));
  ttNameNode->Set(Nan::New(OFX_ATM), NEW_STR_LOCAL("ATM"));
  ttNameNode->Set(Nan::New(OFX_POS), NEW_STR_LOCAL("POS"));
  ttNameNode->Set(Nan::New(OFX_XFER), NEW_STR_LOCAL("XFER"));
  ttNameNode->Set(Nan::New(OFX_CHECK), NEW_STR_LOCAL("CHECK"));
  ttNameNode->Set(Nan::New(OFX_PAYMENT), NEW_STR_LOCAL("PAYMENT"));
  ttNameNode->Set(Nan::New(OFX_CASH), NEW_STR_LOCAL("CASH"));
  ttNameNode->Set(Nan::New(OFX_DIRECTDEP), NEW_STR_LOCAL("DIRECTDEP"));
  ttNameNode->Set(Nan::New(OFX_DIRECTDEBIT), NEW_STR_LOCAL("DIRECTDEBIT"));
  ttNameNode->Set(Nan::New(OFX_REPEATPMT), NEW_STR_LOCAL("REPEATPMT"));
  ttNameNode->Set(Nan::New(OFX_OTHER), NEW_STR_LOCAL("OTHER"));
  exports->Set(NEW_STR_LOCAL("TransactionTypeNames"), ttNameNode);
}

NAN_MODULE_INIT(Init)
{
  Constants(target);
  Nan::Set(target, NEW_STR_LOCAL("parseFile"),
           Nan::GetFunction(Nan::New<v8::FunctionTemplate>(parseFile)).ToLocalChecked());
}

NODE_MODULE(ofx, Init)
