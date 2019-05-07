# Table of Contents

1.  [NodeJS bindings for LibOFX](#orge5741a3)
2.  [Basic Example](#orgbfb2177)
    1.  [List transactions of all accounts in the file](#org1dd97e8)
3.  [API Reference](#orgb54368b)
4.  [Does it miss anything?](#org10f85b0)


<a id="orge5741a3"></a>

# NodeJS bindings for LibOFX

This allows you to parse bank statemetns in formats like OFX & QIF
using `nodejs`. This project only provides the bindings, it's
<https://github.com/libofx/libofx> that does the heavy lifting of
parsing. The library actually does more stuff, but the binding has
limited feature support.


<a id="orgbfb2177"></a>

# Basic Example


<a id="org1dd97e8"></a>

## List transactions of all accounts in the file

    const accounts = libofx.parseFile("/path/to/file.ofx");
    for (const account of accounts) {
      console.log(`Account ${account.info.number}`);
      console.log(`  Balance: ${account.balance.ledger}`);
      console.log(`  Transactions`);
      for (const tr of account.transactions) {
        console.log(`  * ${tr.memo} ${tr.amount} (${tr.datePosted})`);
      }
    }


<a id="orgb54368b"></a>

# API Reference

The library exports one function:

`libofx.parseFile(filePath: string): Array<Account>`.

The `Account` object has the following fields:

    type Account = {
       acctId: string,
       type: libofx.AccountType,
       name: string
       number: string,
       currency: string,
       bankId: string,
       branchId: string,
    }

The field `Account.type` may contain the following values:

-   `libofx.AccountType.CHECKING`
-   `libofx.AccountType.CMA`
-   `libofx.AccountType.CREDITCARD`
-   `libofx.AccountType.CREDITLINE`
-   `libofx.AccountType.INVESTMENT`
-   `libofx.AccountType.MONEYMRKT`
-   `libofx.AccountType.SAVINGS`


<a id="org10f85b0"></a>

# Does it miss anything?

Opening issues and pull requests for fixing or extending the
software are more than welcome!

