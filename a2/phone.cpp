#include <cmath>
#include <forward_list>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

int find_first_bigger_prime(int start) {
  // pre: start > 10
  // post: returns smallest prime that is at least start
  if (start % 2 == 0) {
    start++;
  } // no need to test even numbers
  for (int i = start; i <= (2 * start); i += 2) {
    // By Bertrand's postulate, a prime has to exist in (start, 2*start)
    bool isPrime = true;
    for (int j = 3; j * j <= i; j += 2) {
      if (i % j == 0) {
        isPrime = false;
        break;
      }
    }
    if (isPrime == true) {
      return i;
    }
  }
  return -1;
}

int hash0(long key, int tableSize) { return (int)(key % tableSize); }

int hash1(long key, int tableSize) {
  double phi = (sqrt(5) - 1) / 2;
  double val = key * phi;
  return (int)floor(tableSize * (val - floor(val)));
}

struct DnameEntry {
  std::string name;
  std::string phone;
};

class Dname {
  int M;
  const int R = 255;
  std::vector<std::forward_list<DnameEntry>> table;

  // convert name to a number using the flattening technique and Horner's rule
  int convert_name_to_key(const std::string &name) const {
    int key = 0;
    for (auto c : name) {
      key = hash0((long)key * R + (int)c, M);
    }
    return key;
  }

public:
  /**
   * @brief Constructs a new Dname object with a hash table of given size with
   * chaining
   * @param size Initial size of the hash table (default: 11)
   */
  Dname(int size = 11) : M(size), table(size) {}

  /**
   * @brief Inserts a name-phone entry into the hash table
   * @param entry The DnameEntry containing name and phone number
   * @note Phone numbers for the same name are stored in sorted order
   */
  void insert(const DnameEntry &entry) {
    int key = convert_name_to_key(entry.name);
    auto &chain = table[key];
    auto prev = chain.before_begin();
    for (auto it = chain.begin(); it != chain.end(); ++it) {
      if (it->phone >= entry.phone) {
        break;
      }
      prev = it;
    }
    chain.insert_after(prev, entry);
  }
  /**
   * @brief Searches for all phone numbers associated with a given name
   * @param name The name to search for
   * @return Vector of phone numbers associated with the name, in sorted order
   */
  std::vector<std::string> search(const std::string &name) const {
    std::vector<std::string> result;
    int key = convert_name_to_key(name);
    auto &chain = table[key];
    for (const auto &entry : chain) {
      if (entry.name == name) {
        result.push_back(entry.phone);
      }
    }
    return result;
  }

  /**
   * @brief Doubles the hash table size to the next prime number and rehashes
   * all entries
   */
  void rehash() {
    auto oldtable = std::move(table);
    M = find_first_bigger_prime(2 * M + 1);
    table.assign(M, {});
    for (const auto &chain : oldtable) {
      for (const auto &entry : chain) {
        insert(entry);
      }
    }
  }

  /**
   * @brief Clears the hash table and resets its size to 11
   */
  void clear() {
    M = 11;
    table.assign(M, {});
  }

  friend std::ostream &operator<<(std::ostream &os, const Dname &dname) {
    os << dname.M;
    for (const auto &chain : dname.table) {
      int count = 0;
      for (const auto &entry : chain) {
        ++count;
      }
      os << " " << count;
    }
    os << std::endl;
    return os;
  }
};

struct DphoneEntry {
  std::string phone;
  std::string name;
};

class Dphone {
  int M;
  std::vector<std::optional<DphoneEntry>> table0;
  std::vector<std::optional<DphoneEntry>> table1;

  // convert phone number string to a base 10 integer representation
  long convert_phone(const std::string &phone) const {
    return std::stol(phone.substr(1, 3)) * 10000000 +
           std::stol(phone.substr(5, 3)) * 10000 +
           std::stol(phone.substr(9, 4));
  }

public:
  /**
   * @brief Constructs a new Dphone object with two hash tables of given size
   * for cuckoo hashing
   * @param size Initial size of each hash table (default: 11)
   */
  Dphone(int size = 11) : M(size), table0(size), table1(size) {}

  /**
   * @brief Inserts a phone-name entry using cuckoo hashing with two hash
   * functions
   * @param entry The DphoneEntry containing phone number and name
   * @note Will trigger rehash if insertion cycle exceeds table size
   */
  void insert(DphoneEntry entry) {
    int tableId = 0;
    for (int i = 0; i <= M; ++i) {
      auto &table = (tableId == 0) ? table0 : table1;
      int key = (tableId == 0) ? hash0(convert_phone(entry.phone), M)
                               : hash1(convert_phone(entry.phone), M);
      auto evict = table[key];
      table[key] = entry;
      if (!evict.has_value()) {
        return;
      }

      entry = evict.value();
      tableId = 1 - tableId;
    }

    // rehash and retry if there's no space in the table
    rehash();
    insert(entry);
  }

  /**
   * @brief Searches for a name associated with a given phone number
   * @param phone The phone number to search for
   * @return The associated name or "not found" if not present
   */
  std::string search(const std::string &phone) const {
    // check first table
    auto entry = table0[hash0(convert_phone(phone), M)];
    if (entry && entry.value().phone == phone) {
      return entry.value().name;
    }
    // check second table
    entry = table1[hash1(convert_phone(phone), M)];
    if (entry && entry.value().phone == phone) {
      return entry.value().name;
    }
    return "not found";
  }

  /**
   * @brief Doubles both hash tables' size to the next prime number and rehashes
   * all entries
   */
  void rehash() {
    auto oldTable0 = std::move(table0);
    auto oldTable1 = std::move(table1);
    M = find_first_bigger_prime(2 * M + 1);
    table0.assign(M, {});
    table1.assign(M, {});
    for (auto &entry : oldTable0) {
      if (entry) {
        insert(entry.value());
      }
    }
    for (auto &entry : oldTable1) {
      if (entry) {
        insert(entry.value());
      }
    }
  }

  /**
   * @brief Clears both hash tables and resets their size to 11
   */
  void clear() {
    M = 11;
    table0.assign(M, {});
    table1.assign(M, {});
  }

  friend std::ostream &operator<<(std::ostream &os, const Dphone &dphone) {
    os << dphone.M;
    // print values in first hash table
    for (const auto entry : dphone.table0) {
      os << " " << (entry.has_value() ? "1" : "0");
    }
    // print values in second hash table
    for (const auto entry : dphone.table1) {
      os << " " << (entry.has_value() ? "1" : "0");
    }
    os << std::endl;
    return os;
  }
};

int main() {
  Dname dname;
  Dphone dphone;

  std::string line;
  while (getline(std::cin, line)) {
    std::stringstream conv(line);

    std::string cmd;
    conv >> cmd;

    if (cmd == "i") {
      std::string name, phone;
      conv >> name >> phone;
      dname.insert({name, phone});
      dphone.insert({phone, name});
    } else if (cmd == "l") {
      std::string phone;
      conv >> phone;
      std::cout << dphone.search(phone) << std::endl;
    } else if (cmd == "s") {
      std::string name;
      conv >> name;
      auto results = dname.search(name);
      if (results.empty()) {
        std::cout << "not found" << std::endl;
        continue;
      }
      for (int i = 0; i < results.size(); ++i) {
        if (i) {
          std::cout << ' ';
        }
        std::cout << results[i];
      }
      std::cout << std::endl;
    } else if (cmd == "rh") {
      int table;
      conv >> table;
      if (table == 0) {
        dname.rehash();
      } else {
        dphone.rehash();
      }
    } else if (cmd == "p") {
      int which;
      conv >> which;
      if (which == 0) {
        std::cout << dname;
      } else {
        std::cout << dphone;
      }
    } else if (cmd == "r") {
      dname.clear();
      dphone.clear();
    } else if (cmd == "x") {
      break;
    }
  }

  return 0;
}
