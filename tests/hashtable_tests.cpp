#include "HashTable.h"
#include "catch.hpp"

struct TraitsForString {
    static unsigned hash(const std::string& val) {
        return std::hash<std::string>{}(val);
    }
    static bool equals(const std::string& a, const std::string& b) {
        return a == b;
    }
};

using hashfu::HashTable;
using hashfu::HashTableResult;

using StringTable = HashTable<std::string, TraitsForString>;

TEST_CASE("Construct") {
    struct IntTraits;

    auto table = HashTable<int, IntTraits>{};
    REQUIRE(table.size() == 0);
    REQUIRE(table.empty());
}

TEST_CASE("Populate") {
    StringTable strings;

    strings.insert("One");
    strings.insert("Two");
    strings.insert("Three");

    REQUIRE_FALSE(strings.empty());
    REQUIRE(strings.size() == 3);
}

TEST_CASE("range-for loop") {
    StringTable strings;

    REQUIRE(strings.insert("One") == HashTableResult::InsertedNewEntry);
    REQUIRE(strings.insert("Two") == HashTableResult::InsertedNewEntry);
    REQUIRE(strings.insert("Three") == HashTableResult::InsertedNewEntry);

    int count = 0;
    for (auto& it : strings) {
        REQUIRE_FALSE(it.empty());
        ++count;
    }

    REQUIRE(count == 3);
}

TEST_CASE("Remove") {
    StringTable strings;

    REQUIRE(strings.insert("One") == HashTableResult::InsertedNewEntry);
    REQUIRE(strings.insert("Two") == HashTableResult::InsertedNewEntry);
    REQUIRE(strings.insert("Three") == HashTableResult::InsertedNewEntry);

    REQUIRE(strings.remove("One"));
    REQUIRE(strings.size() == 2u);
    REQUIRE(strings.find("One") == strings.end());

    REQUIRE(strings.remove("Three"));
    REQUIRE(strings.size() == 1u);
    REQUIRE(strings.find("Three") == strings.end());
    REQUIRE(strings.find("Two") != strings.end());
}

TEST_CASE("Case-Insensitive") {
    struct CaseInsensitiveTraits {
        static std::string to_lower(const std::string& val) {
            std::string ret(val);
            std::transform(ret.begin(), ret.end(), ret.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            return ret;
        }
        static unsigned hash(const std::string& val) {
            return std::hash<std::string>{}(to_lower(val));
        }
        static bool equals(const std::string& a, const std::string& b) {
            return to_lower(a) == to_lower(b);
        }
    };

    REQUIRE(CaseInsensitiveTraits::to_lower("HelloWorld") ==
            CaseInsensitiveTraits::to_lower("helloworld"));

    HashTable<std::string, CaseInsensitiveTraits> strings;
    REQUIRE(strings.insert("HelloWorld") == HashTableResult::InsertedNewEntry);

    REQUIRE(strings.insert("helloworld") ==
            HashTableResult::ReplacedExistingEntry);
    REQUIRE(strings.size() == 1u);
}

TEST_CASE("Fuck ton of strings") {
    StringTable strings;
    for (int i = 0; i < 999; ++i) {
        REQUIRE(strings.insert(std::to_string(i)) ==
                HashTableResult::InsertedNewEntry);
    }

    REQUIRE(strings.size() == 999u);
    for (int i = 0; i < 999; ++i) {
        REQUIRE(strings.remove(std::to_string(i)));
    }

    REQUIRE(strings.empty());
}

TEST_CASE("Collisions") {
    // forcing collisions
    struct StringCollisionTraits {
        static unsigned hash(const std::string&) { return 0; }
        static bool equals(const std::string& a, const std::string& b) {
            return a == b;
        }
    };

    HashTable<std::string, StringCollisionTraits> strings;
    for (int i = 0; i < 999; ++i) {
        REQUIRE(strings.insert(std::to_string(i)) ==
                HashTableResult::InsertedNewEntry);
    }

    REQUIRE(strings.insert("foo") == HashTableResult::InsertedNewEntry);
    REQUIRE(strings.size() == 1000u);

    for (int i = 999 - 1; i >= 0; --i) {
        REQUIRE(strings.remove(std::to_string(i)));
    }

    REQUIRE(strings.size() == 1);
    REQUIRE(strings.find("foo") != strings.end());
}

TEST_CASE("Space reuse") {
    struct StringCollisionTraits {
        static unsigned hash(const std::string&) { return 0; }
        static bool equals(const std::string& a, const std::string& b) {
            return a == b;
        }
    };

    HashTable<std::string, StringCollisionTraits> strings;

    // We are checking if the HashTable reuses the space after a few
    // rehashes by removing and inserting an element at the same time
    REQUIRE(strings.insert("0") == HashTableResult::InsertedNewEntry);
    for (int i = 1; i < 5; i++) {
        REQUIRE(strings.insert(std::to_string(i)) ==
                HashTableResult::InsertedNewEntry);
        REQUIRE(strings.remove(std::to_string(i - 1)));
    }

    auto capacity = strings.capacity();

    for (int i = 5; i < 999; ++i) {
        REQUIRE(strings.insert(std::to_string(i)) ==
                HashTableResult::InsertedNewEntry);
        REQUIRE(strings.remove(std::to_string(i - 1)));
    }

    REQUIRE(strings.capacity() == capacity);
}

TEST_CASE("Contains") {
    struct TraitsForInt {
        static unsigned hash(const int& val) { return std::hash<int>{}(val); }
        static bool equals(const int& a, const int& b) { return a == b; }
    };
    HashTable<int, TraitsForInt> table;

    table.insert(1);
    table.insert(2);
    table.insert(3);

    REQUIRE(table.contains(1));
    REQUIRE(table.contains(2));
    REQUIRE(table.contains(3));
    REQUIRE_FALSE(table.contains(4));

    REQUIRE(table.remove(3));
    REQUIRE_FALSE(table.contains(3));
    REQUIRE(table.contains(1));
    REQUIRE(table.contains(2));

    REQUIRE(table.remove(2));
    REQUIRE_FALSE(table.contains(2));
    REQUIRE_FALSE(table.contains(3));
    REQUIRE(table.contains(1));

    REQUIRE(table.remove(1));
    REQUIRE_FALSE(table.contains(1));
}
