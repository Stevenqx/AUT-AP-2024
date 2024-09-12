#include "Bank.h"

#include "Account.h"
#include "Person.h"
#include "Utils.h"

Bank::Bank(const std::string& bank_name, const std::string& bank_fingerprint)
    : bank_name(bank_name),
      hashed_bank_fingerprint(std::hash<std::string>{}(bank_fingerprint)) {}

Bank::~Bank(){}

const std::string& Bank::get_bank_name() const { return bank_name; }