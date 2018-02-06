pragma solidity ^0.4.11;

import "github.com/oraclize/ethereum-api/oraclizeAPI.sol";

contract EthCD_OneHour is usingOraclize {

	uint public constant MINIMUM_DEPOSIT = 0.01 ether;
	uint public constant NUM_PAYOUTS = 30;

  // Each banker account info is stored using the account struct
  struct Account {
  	uint balance;
    uint payouts_left;
    uint payout_amount;
		uint active;
  }

  mapping(address => Account) accountInfo;
  address[] accounts;

  // Total to be paid out (to be displayed on the website)
  uint public total_payout_remaining;

  // Total number of people HODLING (to be displayed on the website)
  uint public total_people_hodling;

	function getBalance() public view returns (uint) {
		return accountInfo[msg.sender].balance;
	}

	function getNumPayoutsLeft() public view returns (uint) {
		return accountInfo[msg.sender].payouts_left;
	}

	function getPayoutAmount() public view returns (uint) {
		return accountInfo[msg.sender].payout_amount;
	}

	function amIActive() public view returns (uint) {
		return accountInfo[msg.sender].active;
	}

  function EthBank() public {
    total_payout_remaining = 0;

		// Start the callbacks 60 seconds later
    oraclize_query(60, "URL", "");
  }

  function deposit() public payable {
    require(msg.value >= MINIMUM_DEPOSIT);

		// We don't worry about whether they have an account.
		// Depositing money just resets the payouts_left and payout_amount variables to default

		if (accountInfo[msg.sender].active != 1) {
   		// Add the sender to the accounts list, set their balance,
			// payout aount, and payouts left
    	accounts.push(msg.sender);
    	accountInfo[msg.sender].balance = msg.value;
    	accountInfo[msg.sender].payouts_left = 30;
    	accountInfo[msg.sender].payout_amount = msg.value / NUM_PAYOUTS;

    	total_payout_remaining += msg.value;
    	total_people_hodling += 1;
		} else {
			// Add to the sender's balance, reset payouts_left to 30,
			// and recalculate payout_aount
			accountInfo[msg.sender].balance += msg.value;
			accountInfo[msg.sender].payouts_left = 30;
			accountInfo[msg.sender].payout_amount = accountInfo[msg.sender].balance / NUM_PAYOUTS;

			total_payout_remaining += msg.value;
		}
  }

  function payout() private {
  	for(uint k = 0; k < accounts.length; k++) {
    	uint transfer_amount = accountInfo[accounts[k]].payout_amount;

			if (transfer_amount > accountInfo[accounts[k]].balance) {
				transfer_amount = accountInfo[accounts[k]].balance;

				// User is no longer hodling, as he is all paid out
				accountInfo[accounts[k]].active = 0;
				total_people_hodling -= 1;
			}

      accounts[k].transfer(transfer_amount);
      accountInfo[accounts[k]].payouts_left -= 1;
      accountInfo[accounts[k]].balance -= transfer_amount;

      total_payout_remaining -= transfer_amount;
    }
  }

  function __callback(bytes32 myid, string result) {
  	if (msg.sender != oraclize_cbAddress()) throw;

    // Recursive oraclize querying - will query every 2 minutes
    oraclize_query(120, "URL", "");
    payout();
 	}
}
