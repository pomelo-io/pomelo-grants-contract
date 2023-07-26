import { TimePointSec, Name } from "@greymass/eosio";
import { Blockchain } from "@proton/vert"
import { describe, beforeEach } from "node:test";
import assert from 'node:assert';

// Vert EOS VM
const blockchain = new Blockchain()

// contracts
const contract = blockchain.createContract('app.pomelo', 'app.pomelo', true);

// blockchain.createAccounts('myaccount', 'anyaccount');

// one-time setup
beforeEach(async () => {
  blockchain.setTime(TimePointSec.from("2023-07-26T00:00:00.000"));
});

function get_history(id) {
  const scope = Name.from('app.pomelo').value.value;
  return contract.tables.devices(scope).getTableRow(BigInt(id));
}

describe('app.pomelo', () => {

  // it("send", async () => {
  //   await contract.actions.send(["myaccount"]).send("anyaccount");
  //   assert.deepEqual(get_history(1).receiver, "myaccount");
  // });

  // it("error: account does not exists", async () => {
  //   const action = contract.actions.send(["invalid"]).send();
  //   await expectToThrow(action, /faucet::send: [to] account does not exist/);
  // });
});

/**
 * Expect a promise to throw an error with a specific message.
 * @param promise - The promise to await.
 * @param {string} errorMsg - The error message that we expect to see.
 */
const expectToThrow = async (promise, errorMsg) => {
  try {
    await promise
    assert.fail('Expected promise to throw an error');
  } catch (e) {
    if ( errorMsg ) assert.match(e.message, errorMsg);
    else assert.fail('Expected promise to throw an error');
  }
}