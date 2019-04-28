// @flow
const libofx = require('..');

describe('libofx', () => {
  it('should fail if file is not found', () => {
    expect(() => libofx.parseFile('/path/to/a/file/that/does/not/exist.ofx'))
      .toThrow(new Error('File not found'));
  });

  it('should read account data from file', () => {
    const accounts = libofx.parseFile('tests/fixtures/justaccount.qfx');
    expect(accounts.length).toBe(1);
    expect(accounts[0].info).toEqual({
      acctId: '003456789 Branch-xxx XXXXXX0000',
      type: libofx.AccountType.CHECKING,
      name: 'Bank account XXXXXX0000',
      number: 'XXXXXX0000',
      currency: 'USD',
      bankId: '003456789',
      branchId: 'Branch-xxx',
    });
  });

  it('should read balance from file', () => {
    const accounts = libofx.parseFile('tests/fixtures/small.qfx');
    expect(accounts.length).toBe(1);
    expect(accounts[0].balance).toEqual({
      ledger: 8381.12,
      ledgerDate: new Date(2017, 2, 27, 11, 14, 49),
    });
  });

  it('should read transactions from file', () => {
    const accounts = libofx.parseFile('tests/fixtures/small.qfx');
    expect(accounts.length).toBe(1);
    expect(accounts[0].transactions).toBeTruthy();
    expect(accounts[0].transactions.length).toBe(8);
    expect(accounts[0].transactions[0]).toEqual({
      amount: -26.86,
      fitId: '201703240001',
      name: 'Debit Card Purchase 03/22 0',
      memo: 'HANA NATURAL           BROOKLYN      NY',
      type: libofx.TransactionType.DEBIT,
      datePosted: new Date(2017, 2, 24, 12, 0, 0),
    });
  });
});
