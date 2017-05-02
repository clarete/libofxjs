// @flow
import bindings from 'bindings';

const ofx = bindings({ bindings: 'ofx', module_root: __dirname });

ofx.transactionTypeName = (type) =>
  ofx.TransactionTypeNames[type].toLowerCase();

export default ofx;
