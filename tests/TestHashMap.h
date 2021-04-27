#include <cxxtest/TestSuite.h>

#include "HashMap.h"

using hashfu::HashMap;
using hashfu::HashTableResult;

class TestHashMap : public CxxTest::TestSuite {
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

   public:
    void test_construct() {
        using IntTable = HashMap<int, int, TraitsForInt>;
        TS_ASSERT(IntTable().is_empty());
        TS_ASSERT_EQUALS(IntTable().size(), 0u);
    }
    void test_populate() {
        HashMap<int, std::string, TraitsForInt> num_to_string;
        num_to_string.insert(1, "one");
        num_to_string.insert(2, "two");
        num_to_string.insert(3, "three");

        TS_ASSERT_EQUALS(num_to_string.is_empty(), false);
        TS_ASSERT_EQUALS(num_to_string.size(), 3u);
    };
    void test_range_loop() {
        HashMap<int, std::string, TraitsForInt> num_to_string;
        TS_ASSERT_EQUALS(num_to_string.insert(1, "one"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(num_to_string.insert(2, "two"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(num_to_string.insert(3, "three"),
                         HashTableResult::InsertedNewEntry);

        int counter = 0;
        for (const auto& it : num_to_string) {
            TS_ASSERT_EQUALS(it.value.empty(), false);
            ++counter;
        }
        TS_ASSERT_EQUALS(counter, 3);
    }
    void test_map_remove() {
        HashMap<int, std::string, TraitsForInt> num_to_string;
        TS_ASSERT_EQUALS(num_to_string.insert(1, "one"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(num_to_string.insert(2, "two"),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(num_to_string.insert(3, "three"),
                         HashTableResult::InsertedNewEntry);

        TS_ASSERT_EQUALS(num_to_string.remove(1), true);
        TS_ASSERT_EQUALS(num_to_string.size(), 2u);
        TS_ASSERT(num_to_string.find(1) == num_to_string.end());

        TS_ASSERT_EQUALS(num_to_string.remove(3), true);
        TS_ASSERT_EQUALS(num_to_string.size(), 1u);
        TS_ASSERT(num_to_string.find(3) == num_to_string.end());
        TS_ASSERT(num_to_string.find(2) != num_to_string.end());
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
        HashMap<std::string, int, CaseInsensitiveTraits> map;
        TS_ASSERT_EQUALS(CaseInsensitiveTraits::to_lower("HelloWorld"),
                         CaseInsensitiveTraits::to_lower("helloworld"));
        TS_ASSERT_EQUALS(map.insert("HelloWorld", 3),
                         HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(map.insert("helloworld", 3),
                         HashTableResult::ReplacedExistingEntry);
        TS_ASSERT_EQUALS(map.size(), 1u);
    }
    void test_fuck_ton_of_strings() {
        HashMap<std::string, int, TraitsForString> strings;

        for (int i = 0; i < 999; ++i) {
            TS_ASSERT_EQUALS(strings.insert(std::to_string(i), i),
                             HashTableResult::InsertedNewEntry);
        }
        TS_ASSERT_EQUALS(strings.size(), 999u);
        for (const auto& it : strings) {
            TS_ASSERT_EQUALS(std::stoi(it.key), it.value);
        }
        for (int i = 998; i >= 0; --i) {
            TS_ASSERT_EQUALS(strings.remove(std::to_string(i)), true);
        }

        TS_ASSERT_EQUALS(strings.is_empty(), true);
    }
    void test_contains() {
        HashMap<int, int, TraitsForInt> map;
        map.insert(1, 10);
        map.insert(2, 20);
        map.insert(3, 30);

        TS_ASSERT_EQUALS(map.contains(1), true);
        TS_ASSERT_EQUALS(map.contains(2), true);
        TS_ASSERT_EQUALS(map.contains(3), true);
        TS_ASSERT_EQUALS(map.contains(4), false);

        TS_ASSERT_EQUALS(map.remove(3), true);
        TS_ASSERT_EQUALS(map.contains(3), false);
        TS_ASSERT_EQUALS(map.contains(1), true);
        TS_ASSERT_EQUALS(map.contains(2), true);

        TS_ASSERT_EQUALS(map.remove(2), true);
        TS_ASSERT_EQUALS(map.contains(2), false);
        TS_ASSERT_EQUALS(map.contains(3), false);
        TS_ASSERT_EQUALS(map.contains(1), true);

        TS_ASSERT_EQUALS(map.remove(1), true);
        TS_ASSERT_EQUALS(map.contains(1), false);
    }
    void test_array_subscript() {
        // testing operator[] overload
        HashMap<std::string, int, TraitsForString> counts;

        auto word_list = {"this", "not", "this", "bye", "not"};
        for (const auto& word : word_list) {
            ++counts[word];
        }

        TS_ASSERT_EQUALS(counts.find("this")->value, 2);
        TS_ASSERT_EQUALS(counts.find("not")->value, 2);
        TS_ASSERT_EQUALS(counts.find("bye")->value, 1);
    }
};
