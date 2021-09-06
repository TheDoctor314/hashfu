#include "HashMap.h"
#include "catch.hpp"

struct TraitsForInt {
    static unsigned hash(const int& a) { return std::hash<int>{}(a); }
    static bool equals(const int& a, const int& b) { return a == b; }
};

struct TraitsForString {
    static unsigned hash(const std::string& val) {
        return std::hash<std::string>{}(val);
    }
    static bool equals(const std::string& a, const std::string& b) {
        return a == b;
    }
};

using hashfu::HashMap;
using hashfu::HashTableResult;

using IntTable = HashMap<int, std::string, TraitsForInt>;
using StringTable = HashMap<std::string, int, TraitsForString>;

TEST_CASE("Construct") {
    using IntTable = HashMap<int, int, TraitsForInt>;
    REQUIRE(IntTable().is_empty());
    REQUIRE(IntTable().size() == 0u);
}

TEST_CASE("Populate") {
    IntTable num_to_string;
    num_to_string.insert(1, "one");
    num_to_string.insert(2, "two");
    num_to_string.insert(3, "three");

    REQUIRE_FALSE(num_to_string.is_empty());
    REQUIRE(num_to_string.size() == 3u);
}

TEST_CASE("Range-for loop") {
    IntTable num_to_string;
    REQUIRE(num_to_string.insert(1, "one") ==
            HashTableResult::InsertedNewEntry);
    REQUIRE(num_to_string.insert(2, "two") ==
            HashTableResult::InsertedNewEntry);
    REQUIRE(num_to_string.insert(3, "three") ==
            HashTableResult::InsertedNewEntry);

    int counter = 0;
    for (const auto& it : num_to_string) {
        REQUIRE_FALSE(it.value.empty());
        ++counter;
    }

    REQUIRE(counter == 3);
}

/*
TEST_CASE("Remove") {
    IntTable num_to_string;
    REQUIRE(num_to_string.insert(1, "one") ==
            HashTableResult::InsertedNewEntry);
    REQUIRE(num_to_string.insert(2, "two") ==
            HashTableResult::InsertedNewEntry);
    REQUIRE(num_to_string.insert(3, "three") ==
            HashTableResult::InsertedNewEntry);

    REQUIRE(num_to_string.remove(1));
    REQUIRE(num_to_string.size() == 2u);
    REQUIRE(num_to_string.find(1) == num_to_string.end());

    REQUIRE(num_to_string.remove(3));
    REQUIRE(num_to_string.size() == 1u);
    REQUIRE(num_to_string.find(3) == num_to_string.end());
    REQUIRE(num_to_string.find(2) != num_to_string.end());
}
*/

TEST_CASE("Case insensitive") {
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

    HashMap<std::string, int, CaseInsensitiveTraits> map;
    REQUIRE(CaseInsensitiveTraits::to_lower("HelloWorld") ==
            CaseInsensitiveTraits::to_lower("helloworld"));

    REQUIRE(map.insert("HelloWorld", 3) == HashTableResult::InsertedNewEntry);
    REQUIRE(map.insert("helloworld", 3) ==
            HashTableResult::ReplacedExistingEntry);
    REQUIRE(map.size() == 1u);
}

TEST_CASE("Fuck ton of strings") {
    StringTable strings;

    for (int i = 0; i < 999; ++i) {
        REQUIRE(strings.insert(std::to_string(i), i) ==
                HashTableResult::InsertedNewEntry);
    }

    REQUIRE(strings.size() == 999u);

    for (const auto& it : strings) {
        REQUIRE(std::stoi(it.key) == it.value);
    }

    for (int i = 998; i >= 0; --i) {
        REQUIRE(strings.remove(std::to_string(i)));
    }

    REQUIRE(strings.is_empty());
}

TEST_CASE("Contains") {
    HashMap<int, int, TraitsForInt> map;
    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);

    REQUIRE(map.contains(1));
    REQUIRE(map.contains(2));
    REQUIRE(map.contains(3));
    REQUIRE_FALSE(map.contains(4));

    REQUIRE(map.remove(3));
    REQUIRE_FALSE(map.contains(3));
    REQUIRE(map.contains(1));
    REQUIRE(map.contains(2));

    REQUIRE(map.remove(2));
    REQUIRE_FALSE(map.contains(2));
    REQUIRE_FALSE(map.contains(3));
    REQUIRE(map.contains(1));

    REQUIRE(map.remove(1));
    REQUIRE_FALSE(map.contains(1));
}

TEST_CASE("Array subscript") {
    // testing operator[] overload

    StringTable counts;

    const auto word_list = {"this", "not", "this", "bye", "not"};
    for (const auto& word : word_list) {
        ++counts[word];
    }

    REQUIRE(counts.find("this")->value == 2);
    REQUIRE(counts.find("not")->value == 2);
    REQUIRE(counts.find("bye")->value == 1);
}
