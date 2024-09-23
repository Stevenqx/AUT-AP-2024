#include "Bank.h"

#include <algorithm>
#include <stdexcept>

#include "Account.h"
#include "Person.h"
#include "Utils.h"

Bank::Bank(const std::string& bank_name, const std::string& bank_fingerprint)
    : bank_name(bank_name),
      hashed_bank_fingerprint(std::hash<std::string>{}(bank_fingerprint)),
      bank_customers(),
      bank_accounts(),
      account_2_customer(),
      customer_2_accounts(),
      customer_2_paid_loan(),
      customer_2_unpaid_loan(),
      bank_total_balance(0),
      bank_total_loan(0) {}

Bank::~Bank() {
  for (auto& account_p : bank_accounts) {
    delete account_p;
  }
}

Account* Bank::create_account(Person& owner,
                              const std::string& owner_fingerprint,
                              std::string password) {
  if (!authenticate_owner(owner, owner_fingerprint)) 
    throw std::logic_error("Owner authentication fails!");
  auto account_p = new Account(&owner, this, password);
  bank_accounts.push_back(account_p);
  account_2_customer[account_p] = &owner;

  if (std::find(bank_customers.begin(), bank_customers.end(), &owner) ==
      bank_customers.end()) {
    bank_customers.push_back(&owner);
    customer_2_accounts[&owner] = std::vector<Account*>();
  }
  customer_2_accounts[&owner].push_back(account_p);

  return account_p;
}

bool Bank::delete_account(Account& account,
                          const std::string& owner_fingerprint) {
  if (!authenticate_owner(account.owner, owner_fingerprint)) {
    throw std::logic_error("Owner authentication fails!");
  }
  Account* account_p = &account;
  Person* owner_p = account_2_customer[account_p];
  auto loan_it = customer_2_unpaid_loan.find(owner_p);
  if (loan_it != customer_2_unpaid_loan.end()) {
    throw std::logic_error("This customer stills has unpaid loan!");
  }  

  bank_accounts.erase(
      std::find(bank_accounts.begin(), bank_accounts.end(), account_p));
  account_2_customer.erase(account_p);
  auto& accounts = customer_2_accounts[owner_p];
  accounts.erase(std::remove(accounts.begin(), accounts.end(), account_p),
                 accounts.end());

  delete account_p;
  return true;
}

bool Bank::delete_customer(Person& owner,
                           const std::string& owner_fingerprint) {
  if (!authenticate_owner(owner, owner_fingerprint)) {
    throw std::logic_error("Owner authentication fails!");
  }
  Person* owner_p = &owner;
  auto loan_it = customer_2_unpaid_loan.find(owner_p);
  if (loan_it != customer_2_unpaid_loan.end()) {
    throw std::logic_error("This customer stills has unpaid loan!");
  }  

  bank_customers.erase(
      std::find(bank_customers.begin(), bank_customers.end(), owner_p));
  auto& accounts = customer_2_accounts[owner_p];
  for (auto& account_p : accounts) {
    bank_accounts.erase(
        std::find(bank_accounts.begin(), bank_accounts.end(), account_p));
    account_2_customer.erase(account_p);
    delete account_p;
  }
  customer_2_accounts.erase(owner_p);

  customer_2_paid_loan.erase(owner_p);
  customer_2_unpaid_loan.erase(owner_p);

  // Don't "delete" the person!
  return true;
}

bool Bank::deposit(Account& account, const std::string& owner_fingerprint,
                   double amount) {
  if (!authenticate_owner(account.owner, owner_fingerprint)) {
    throw std::logic_error("Owner authentication fails!");
  }
  account.balance += amount;
  return true;
}

bool Bank::withdraw(Account& account, const std::string& owner_fingerprint,
                    double amount) {
  if (!authenticate_owner(account.owner, owner_fingerprint))
    throw std::logic_error("Owner authentication fails!");
  if (account.balance < amount)
    throw std::logic_error("The balance is not sufficient!");
  account.balance -= amount;
  return true;
}

bool Bank::transfer(Account& source, Account& destination,
                    const std::string& owner_fingerprint,
                    const std::string& CVV2, const std::string& password,
                    const std::string& exp_date, double amount) {
  if (!authenticate_owner(source.owner, owner_fingerprint))
    throw std::logic_error("Owner authentication fails!");
  if (CVV2 != source.CVV2) return false;
  if (password != source.password) return false;
  if (exp_date != source.exp_date) return false;
  if (source.balance < amount) return false;
  source.balance -= amount;
  destination.balance += amount;
  return true;
}

bool Bank::take_loan(Account& account, const std::string& owner_fingerprint,
                     double amount) {
  Person* owner = account_2_customer[&account];
  if (!authenticate_owner(owner, owner_fingerprint))
    throw std::logic_error("Owner authentication fails!");
  double total_balance = 0;
  for (const auto account : customer_2_accounts[owner]) {
    total_balance += account->balance;
  }
  double interest = amount / owner->get_socioeconomic_rank() / 10;
  double total_amount = amount + interest;

  double loan_limit =
      (owner->get_socioeconomic_rank() * 1.0 / 10) * total_balance;
  auto unpaid_loan_iter = customer_2_unpaid_loan.find(owner);
  bool has_unpaid_loan = unpaid_loan_iter != customer_2_unpaid_loan.end();
  double unpaid_loan = has_unpaid_loan ? unpaid_loan_iter->second : 0;

  if (unpaid_loan + total_amount > loan_limit) 
    throw std::logic_error("Insufficient eligibility!");
  if (has_unpaid_loan) {
    unpaid_loan_iter->second += total_amount;
  } else {
    customer_2_unpaid_loan[owner] = total_amount;
  }
  bank_total_loan += total_amount;
  bank_total_balance += interest;
  return true;
}

bool Bank::pay_loan(Account& account, double amount) {
  Person* owner_p = account_2_customer[&account];
  auto unpaid_loan_iter = customer_2_unpaid_loan.find(owner_p);
  if (unpaid_loan_iter == customer_2_unpaid_loan.end()) return false;
  unpaid_loan_iter->second -= amount;
  bank_total_loan -= amount;

  auto paid_loan_iter = customer_2_paid_loan.find(owner_p);
  if (paid_loan_iter == customer_2_paid_loan.end()) {
    customer_2_paid_loan[owner_p] = amount;
  } else {
    customer_2_paid_loan[owner_p] += amount;
  }

  // update socioeconomic rank
  double paid_loan = customer_2_paid_loan[owner_p];
  size_t new_rank = 1;
  for (; new_rank <= 10 && paid_loan >= 10; new_rank++) {
    paid_loan = paid_loan / 10;
  }
  if (new_rank > 10) new_rank = 10;
  owner_p->set_socioeconomic_rank(new_rank);
  return true;
}

const std::string& Bank::get_bank_name() const { return bank_name; }

size_t Bank::get_hashed_bank_fingerprint() const {
  return hashed_bank_fingerprint;
}

const std::vector<Person*>& Bank::get_bank_customers(
    std::string& bank_fingerprint) const {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  return bank_customers;
}

const std::vector<Account*>& Bank::get_bank_accounts(
    std::string& bank_fingerprint) const {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  return bank_accounts;
}

const std::map<Account*, Person*>& Bank::get_account_2_customer_map(
    std::string& bank_fingerprint) const {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  return account_2_customer;
}

const std::map<Person*, std::vector<Account*>>&
Bank::get_customer_2_accounts_map(std::string& bank_fingerprint) const {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  return customer_2_accounts;
}

const std::map<Person*, double>& Bank::get_customer_2_paid_loan_map(
    std::string& bank_fingerprint) const {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  return customer_2_paid_loan;
}

const std::map<Person*, double>& Bank::get_customer_2_unpaid_loan_map(
    std::string& bank_fingerprint) const {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  return customer_2_unpaid_loan;
}

double Bank::get_bank_total_balance(std::string& bank_fingerprint) const {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  return bank_total_balance;
}

double Bank::get_bank_total_loan(std::string& bank_fingerprint) const {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  return bank_total_loan;
}

bool Bank::set_owner(Account& account, const Person* new_owner,
                     std::string& owner_fingerprint,
                     std::string& bank_fingerprint) {
  if (!authenticate_owner(account.owner, owner_fingerprint)) {
    throw std::logic_error("Original owner authentication fails!");
  }
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }

  auto new_owner_iter =
      std::find(bank_customers.begin(), bank_customers.end(), new_owner);
  Person* original_owner = account_2_customer[&account];
  if (new_owner_iter == bank_customers.end()) return false;
  account_2_customer[&account] = *new_owner_iter;

  auto& original_accounts = customer_2_accounts[original_owner];
  original_accounts.erase(
      std::remove(original_accounts.begin(), original_accounts.end(), &account),
      original_accounts.end());
  customer_2_accounts[*new_owner_iter].push_back(&account);

  account.owner = new_owner;

  return true;
}

bool Bank::set_account_status(Account& account, bool status,
                              std::string& bank_fingerprint) {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  account.account_status = status;
  return true;
}

bool Bank::set_exp_date(Account& account, std::string& exp_date,
                        std::string& bank_fingerprint) {
  if (!authenticate_bank(bank_fingerprint)) {
    throw std::logic_error("Bank authentication fails!");
  }
  account.exp_date = exp_date;
  return true;
}

// void Bank::get_info(std::optional<std::string> file_name = std::nullopt)
// const {
// }

bool Bank::authenticate_owner(const Person& owner,
                              const std::string& fingerprint) const {
  if (std::hash<std::string>{}(fingerprint) == owner.get_hashed_fingerprint())
    return true;
  else
    return false;
}

bool Bank::authenticate_owner(const Person* const owner,
                              const std::string& fingerprint) const {
  if (std::hash<std::string>{}(fingerprint) == owner->get_hashed_fingerprint())
    return true;
  else
    return false;
}

bool Bank::authenticate_bank(const std::string& fingerprint) const {
  if (std::hash<std::string>{}(fingerprint) == hashed_bank_fingerprint)
    return true;
  else
    return false;
}