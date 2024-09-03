#include "Person.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Utils.h"

Person::Person(std::string &name, size_t age, std::string &gender,
               std::string &fingerprint, size_t socioeconomic_rank,
               bool is_alive)
    : name(name),
      age(age),
      gender(gender),
      hashed_fingerprint(std::hash<std::string>{}(fingerprint)),
      socioeconomic_rank(socioeconomic_rank),
      is_alive(is_alive) {
  if (age <= 0) throw std::invalid_argument("The age must be greater than 0.");
  if (gender != "Female" && gender != "Male")
    throw std::invalid_argument("The gender must be Male or Female.");
  if (socioeconomic_rank < 1 || socioeconomic_rank > 10)
    throw std::invalid_argument(
        "The socioeconomic rank is outside the range 1-10.");
}

std::string Person::get_name() const { return name; }
size_t Person::get_age() const { return age; }
std::string Person::get_gender() const { return gender; }
size_t Person::get_hashed_fingerprint() const { return hashed_fingerprint; }
size_t Person::get_socioeconomic_rank() const { return socioeconomic_rank; }
bool Person::get_is_alive() const { return is_alive; }

bool Person::set_age(size_t age) {
  if (age <= 0) throw std::invalid_argument("The age must be greater than 0.");
  this->age = age;
  return true;
}
bool Person::set_socioeconomic_rank(size_t rank) {
  if (rank < 1 || rank > 10)
    throw std::invalid_argument(
        "The socioeconomic rank is outside the range 1-10.");

  this->socioeconomic_rank = rank;
  return true;
}
bool Person::set_is_alive(bool is_alive) {
  this->is_alive = is_alive;
  return true;
}

std::strong_ordering Person::operator<=>(const Person &other) const {
  return hashed_fingerprint <=> other.hashed_fingerprint;
}

void Person::get_info(std::optional<std::string> file_name) const {
  std::ostream *output_p = &std::cout;
  std::ofstream out_file;

  if (file_name.has_value()) {
    out_file.open(file_name.value());
    output_p = &out_file;
  }

  *output_p << "Personal information:\nName:\t" << name << "\nAge:\t" << age
            << "\nGender:\t" << gender << "\nHashed fingerprint:\t"
            << hashed_fingerprint << "\nSocioeconomic rank:\t"
            << socioeconomic_rank << "\nIs alive:\t" << is_alive << "\n";
}