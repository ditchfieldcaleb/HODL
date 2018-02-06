pragma solidity ^0.4.11;

contract EthCD_OneHour {

	uint public constant MINIMUM_DEPOSIT = 0.01 ether;	// Min payout to avoid weirdness
	uint public constant NUM_PAYOUTS = 60; 							// 60 minutes in an hour
	address public WHALE = 0x1e0dcc50C15581c4aD9CaC663A8283DACcA53271;

  // Each banker account info is stored using the account struct
  struct Account {
  	uint balance;
    uint payouts_left;
    uint payout_amount;
		uint active;
		uint created;
  }

  mapping(address => Account) accountInfo;

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

  function EthCD_OneHour() public {
    total_payout_remaining = 0;
  }

  function deposit() public payable {
    require(msg.value >= MINIMUM_DEPOSIT);

		// We don't worry about whether they have an account.
		// Depositing money just resets the payouts_left and payout_amount variables to default

		if (accountInfo[msg.sender].active != 1) {
   		// Add the sender to the accounts list, set their balance,
			// payout aount, and payouts left
    	accountInfo[msg.sender].balance = msg.value;
    	accountInfo[msg.sender].payouts_left = 60;
    	accountInfo[msg.sender].payout_amount = msg.value / NUM_PAYOUTS;
			accountInfo[msg.sender].active = 1;
			accountInfo[msg.sender].created = now;

    	total_payout_remaining += msg.value;
    	total_people_hodling += 1;
		} else {
			// Add to the sender's balance, reset payouts_left to 30,
			// and recalculate payout_aount (and reset the created date)
			accountInfo[msg.sender].balance += msg.value;
			accountInfo[msg.sender].payouts_left = 60;
			accountInfo[msg.sender].payout_amount = accountInfo[msg.sender].balance / NUM_PAYOUTS;
			accountInfo[msg.sender].created = now;

			total_payout_remaining += msg.value;
		}
  }

	// Panic sell. Painful. Loses 10% of the value instantly. Just don't.
	function panic_sell() public {
		require(accountInfo[msg.sender].active == 1);

		uint remaining_balance = accountInfo[msg.sender].balance;
		uint whale_profit = remaining_Balance / 10;
		uint amount_to_panic_seller = remaining_balance - whale_profit;

		accountInfo[msg.sender].balance = 0;
		accountInfo[msg.sender].active = 0;

		WHALE.transfer(whale_profit);
		msg.sender.transfer(amount_to_panic_seller);
	}

	// Normal payout function. Drip-style, no penalty. Incentivices hodling.
  function payout() public {
		require(accountInfo[msg.sender].active == 1);

		uint seconds_since_created = now - accountInfo[msg.sender].created;
		uint minutes_since_created = seconds_since_created / 60;

		// Calculate how much balance the payer should have remaining in his account at this point
		// Payout the difference
		//
		// Example: should have 20 payouts left. Actually has 26 payouts left.
		// thus we payout 6 payouts.
		uint remaining_payouts = NUM_PAYOUTS - (minutes_since_created);

		uint payouts_to_give = accountInfo[msg.sender].payouts_left - remaining_payouts;

		// Sanity check! If it's totally expired, just give the rest of the payouts
		if (minutes_since_created >= NUM_PAYOUTS) {
			payouts_to_give = accountInfo[msg.sender].payouts_left;
		}

		accountInfo[msg.sender].payouts_left -= payouts_to_give;
		uint transfer_amount = payouts_to_give * accountInfo[msg.sender].payout_amount;
		accountInfo[msg.sender].balance -= transfer_amount;
		total_payout_remaining -= transfer_amount;

		if (accountInfo[msg.sender].payouts_left == 0) {
			accountInfo[msg.sender].active = 0;
		}

		// Sanity check
		assert(accountInfo[msg.sender].balance >= transfer_amount);

		msg.sender.transfer(transfer_amount);
  }
}
