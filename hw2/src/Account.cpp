#include "Account.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Bank.h"
#include "Person.h"
#include "Utils.h"

int Account::account_number_generator = 0;

Account::Account(const Person* const owner, const Bank* const bank,
                 std::string& password)
    : owner(owner),
      bank(bank),
      account_number(std::to_string((long long)(1e15) + account_number_generator++)),
      balance(0),
      account_status(true),
      CVV2("1234"),
      password(password),
      exp_date("30-01") {}

const Person* Account::get_owner() const { return owner; }
double Account::get_balance() const { return balance; }
std::string Account::get_account_number() const { return account_number; }
bool Account::get_status() const { return account_status; }

std::string Account::get_CVV2(std::string& owner_fingerprint) const {
  authenticate(owner_fingerprint);
  return CVV2;
}
std::string Account::get_password(std::string& owner_fingerprint) const {
  authenticate(owner_fingerprint);
  return password;
}
std::string Account::get_exp_date(std::string& owner_fingerprint) const {
  authenticate(owner_fingerprint);
  return exp_date;
}

bool Account::set_password(std::string& password,
                           std::string& owner_fingerprint) {
  authenticate(owner_fingerprint);
  this->password = password;
  return true;
}

std::strong_ordering Account::operator<=>(const Account& other) const {
  return account_number <=> other.account_number;
}

void Account::get_info(std::optional<std::string> file_name) const {
  std::ostream* output_p = &std::cout;
  std::ofstream out_file;

  if (file_name.has_value()) {
    out_file.open(file_name.value());
    output_p = &out_file;
  }

  *output_p << "Account information:\nOwner:\t" << owner->get_name()
            << "\nBank:\t" << bank->get_bank_name() << "\nAccount number:\t"
            << account_number << "\nBalance:\t" << balance
            << "\nAccount status:\t" << account_status << "\n";
}

void Account::authenticate(const std::string& owner_fingerprint) const {
  if (std::hash<std::string>{}(owner_fingerprint) !=
      owner->get_hashed_fingerprint())
    throw std::logic_error("Authentication fails!");
}