solidity ^0.4.11;

import "github.com/oraclize/ethereum-api/oraclizeAPI.sol";

contract EthBank is usingOraclize {

	uint public constant MINIMUM_DEPOSIT = 0.01 ether;

  // Each banker account info is stored using the account struct
  struct Account {
  	uint balance;
    uint payout_interval;
    uint payouts_left;
    uint payout_amount;
  }

  mapping(address => Account) accountInfo;
  address[] accounts;

  // Total to be paid out (to be displayed on the website)
  uint public total_payout_remaining;

  // Total number of people HODLING (to be displayed on the website)
  uint public total_people_hodling;

  function EthBank() public {
    total_payout_remaining = 0;
    oraclize_query(60, "URL", "");
  }

  function deposit(uint interval, uint num_payouts) public payable {
    require(interval > 60);
    require(num_payouts > 0);
    require(msg.value >= MINIMUM_DEPOSIT);

    // Add the sender to the accounts list
    // Also add the
    accounts.push(msg.sender);
    accountInfo[msg.sender].balance += msg.value;
    accountInfo[msg.sender].payout_interval = interval;
    accountInfo[msg.sender].payouts_left = num_payouts;
    accountInfo[msg.sender].payout_amount = msg.value / num_payouts;

    total_payout_remaining += msg.value;
    total_people_hodling += 1;
  }

  function payout() private {
  	for(uint k = 0; k < accounts.length; k++) {
    	uint transfer_amount = accountInfo[accounts[k]].payout_amount;

			if (transfer_amount > accountInfo[accounts[k]].balance) {
				transfer_amount = accountINfo[accounts[k]].balance;
			}

      accounts[k].transfer(transfer_amount);
      accountInfo[accounts[k]].payouts_left -= 1;
      accountInfo[accounts[k]].balance -= 1;

      total_payout_remaining -= transfer_amount;
    }
  }

  function __callback(bytes32 myid, string result) {
  	if (msg.sender != oraclize_cbAddress()) throw;

    // Recursive oraclize querying - will query every 24 hours
    oraclize_query(120, "URL", "");
    payout();
 	}
}
