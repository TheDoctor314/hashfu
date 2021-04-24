#include <cxxtest/TestSuite.h>

#include <functional>
#include <string>

#include "../HashTable.h"

using hashfu::HashTable;

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
    void testConstruct() {
        struct TraitsForInt;
        using IntTable = HashTable<int, TraitsForInt>;
        TS_ASSERT_EQUALS(IntTable().size(), 0u);
        TS_ASSERT(IntTable().is_empty());
    }
    void testPopulate() {
        HashTable<std::string, TraitsForString> strings;
        strings.insert("One");
        strings.insert("Two");
        strings.insert("Three");

        TS_ASSERT_EQUALS(strings.is_empty(), false);
        TS_ASSERT_EQUALS(strings.size(), 3);
    }
    void testRangeLoop() {
        HashTable<std::string, TraitsForString> strings;
        TS_ASSERT_EQUALS(strings.insert("One"),
                         hashfu::HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(strings.insert("Two"),
                         hashfu::HashTableResult::InsertedNewEntry);
        TS_ASSERT_EQUALS(strings.insert("Three"),
                         hashfu::HashTableResult::InsertedNewEntry);

        int count = 0;
        for (auto& it : strings) {
            TS_ASSERT_EQUALS(it.empty(), false);
            ++count;
        }
        TS_ASSERT_EQUALS(count, 3);
    }
};