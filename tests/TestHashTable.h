#include <cxxtest/TestSuite.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>

#include "HashTable.h"

using hashfu::HashTable;
using hashfu::HashTableResult;

class TestHashTable : public CxxTest::TestSuite {
    struct TraitsForString {
        static unsigned hash(const std::string& val) {
            return std::hash<std::string>{}(val);
        }
        static bool equals(const std::string& a, const std::string& b) {
            return a == b;
        }
    };

   public:
    void test_construct() {
        struct TraitsForInt;
        using IntTable = HashTable<int, TraitsForInt>;
        TS_ASSERT_EQUALS(IntTable().size(), 0u);
        TS_ASSERT(IntTable().is_empty());
    }
    void test_populate() {
        HashTable<std::string, TraitsForString> strings;
        strings.insert("One");
        strings.insert("Two");
        strings.insert("Three");

        TS_ASSERT_EQUALS(strings.is_empty(), false);
        TS_ASSERT_EQUALS(strings.size(), 3);
    }
    void test_range_loop() {
        HashTable<std::string, TraitsForString> strings;
        TS_ASSERT_EQUALS(strings.insert("One"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(strings.insert("Two"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(strings.insert("Three"),
                         HashTableResult::InsertedNewEntry);

        int count = 0;
        for (auto& it : strings) {
            TS_ASSERT_EQUALS(it.empty(), false);
            ++count;
        }
        TS_ASSERT_EQUALS(count, 3);
    }
    void test_remove() {
        HashTable<std::string, TraitsForString> strings;
        TS_ASSERT_EQUALS(strings.insert("One"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(strings.insert("Two"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(strings.insert("Three"),
                         HashTableResult::InsertedNewEntry);

        TS_ASSERT_EQUALS(strings.remove("One"), true);
        TS_ASSERT_EQUALS(strings.size(), 2u);
        TS_ASSERT(strings.find("One") == strings.end());

        TS_ASSERT_EQUALS(strings.remove("Three"), true);
        TS_ASSERT_EQUALS(strings.size(), 1u);
        TS_ASSERT(strings.find("Three") == strings.end());
        TS_ASSERT(strings.find("Two") != strings.end());
    }
    void test_case_insensitive() {
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

        TS_ASSERT_EQUALS(CaseInsensitiveTraits::to_lower("HelloWorld"),
                         CaseInsensitiveTraits::to_lower("helloworld"));

        HashTable<std::string, CaseInsensitiveTraits> strings;
        TS_ASSERT_EQUALS(strings.insert("HelloWorld"),
                         HashTableResult::InsertedNewEntry);

        TS_ASSERT_EQUALS(strings.insert("helloworld"),
                         HashTableResult::ReplacedExistingEntry);
        TS_ASSERT_EQUALS(strings.size(), 1u);
    }
    void test_shit_ton_of_strings() {
        HashTable<std::string, TraitsForString> strings;
        for (int i = 0; i < 999; ++i) {
            TS_ASSERT_EQUALS(strings.insert(std::to_string(i)),
                             HashTableResult::InsertedNewEntry);
        }

        TS_ASSERT_EQUALS(strings.size(), 999u);
        for (int i = 0; i < 999; ++i) {
            TS_ASSERT_EQUALS(strings.remove(std::to_string(i)), true);
        }

        TS_ASSERT(strings.is_empty());
    }
    void test_many_collisions() {
        // forcing collisions
        struct StringCollisionTraits {
            static unsigned hash(const std::string& val) { return 0; }
            static bool equals(const std::string& a, const std::string& b) {
                return a == b;
            }
        };

        HashTable<std::string, StringCollisionTraits> strings;
        for (int i = 0; i < 999; ++i) {
            TS_ASSERT_EQUALS(strings.insert(std::to_string(i)),
                             HashTableResult::InsertedNewEntry);
        }

        TS_ASSERT_EQUALS(strings.insert("foo"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(strings.size(), 1000u);

        for (int i = 999 - 1; i >= 0; --i) {
            TS_ASSERT_EQUALS(strings.remove(std::to_string(i)), true);
        }

        TS_ASSERT_EQUALS(strings.size(), 1);
        TS_ASSERT(strings.find("foo") != strings.end());
    }
    void test_space_reuse() {
        struct StringCollisionTraits {
            static unsigned hash(const std::string& val) { return 0; }
            static bool equals(const std::string& a, const std::string& b) {
                return a == b;
            }
        };

        HashTable<std::string, StringCollisionTraits> strings;

        // We are checking if the HashTable reuses the space after a few
        // rehashes by removing and inserting an element at the same time
        TS_ASSERT_EQUALS(strings.insert("0"),
                         HashTableResult::InsertedNewEntry);
        for (int i = 1; i < 5; i++) {
            TS_ASSERT_EQUALS(strings.insert(std::to_string(i)),
                             HashTableResult::InsertedNewEntry);
            TS_ASSERT_EQUALS(strings.remove(std::to_string(i - 1)), true);
        }

        auto capacity = strings.capacity();

        for (int i = 5; i < 999; ++i) {
            TS_ASSERT_EQUALS(strings.insert(std::to_string(i)),
                             HashTableResult::InsertedNewEntry);
            TS_ASSERT_EQUALS(strings.remove(std::to_string(i - 1)), true);
        }

        TS_ASSERT_EQUALS(strings.capacity(), capacity);
    }
    void test_contains() {
        struct TraitsForInt {
            static unsigned hash(const int& val) {
                return std::hash<int>{}(val);
            }
            static bool equals(const int& a, const int& b) { return a == b; }
        };
        HashTable<int, TraitsForInt> table;

        table.insert(1);
        table.insert(2);
        table.insert(3);

        TS_ASSERT_EQUALS(table.contains(1), true);
        TS_ASSERT_EQUALS(table.contains(2), true);
        TS_ASSERT_EQUALS(table.contains(3), true);
        TS_ASSERT_EQUALS(table.contains(4), false);

        TS_ASSERT_EQUALS(table.remove(3), true);
        TS_ASSERT_EQUALS(table.contains(3), false);
        TS_ASSERT_EQUALS(table.contains(1), true);
        TS_ASSERT_EQUALS(table.contains(2), true);

        TS_ASSERT_EQUALS(table.remove(2), true);
        TS_ASSERT_EQUALS(table.contains(2), false);
        TS_ASSERT_EQUALS(table.contains(3), false);
        TS_ASSERT_EQUALS(table.contains(1), true);

        TS_ASSERT_EQUALS(table.remove(1), true);
        TS_ASSERT_EQUALS(table.contains(1), false);
    }
};
