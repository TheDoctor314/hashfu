#pragma once

#include <cstddef>

#include "HashTable.h"

namespace hashfu {
template <typename K, typename V, typename KeyTraits>
class HashMap {
    struct Entry {
        K key;
        V value;
    };
    struct EntryTraits {
        static unsigned hash(const Entry& e) { return KeyTraits::hash(e.key); }
        static bool equals(const Entry& a, const Entry& b) {
            return KeyTraits::equals(a.key, b.key);
        }
    };
    using HashTableType = HashTable<Entry, EntryTraits>;
    using IteratorType = typename HashTableType::Iterator;
    using ConstIteratorType = typename HashTableType::ConstIterator;

   private:
    HashTableType table_;

   public:
    HashMap() = default;

    bool is_empty() const { return table_.is_empty(); }
    size_t size() const { return table_.size(); }
    size_t capacity() const { return table_.capacity(); }
    float load_factor() const { return table_.load_factor(); }

    void clear() { table_.clear(); }

    ConstIteratorType begin() const { return table_.begin(); }
    ConstIteratorType end() const { return table_.end(); }
    ConstIteratorType find(const K& key) const {
        return table_.find(KeyTraits::hash(key), [&](auto& entry) {
            return KeyTraits::equals(key, entry.key);
        });
    }
    template <typename Pred>
    ConstIteratorType find(unsigned hash, Pred predicate) const {
        return table_.find(hash, predicate);
    }

    IteratorType begin() { return table_.begin(); }
    IteratorType end() { return table_.end(); }
    IteratorType find(const K& key) {
        return table_.find(KeyTraits::hash(key), [&](auto& entry) {
            return KeyTraits::equals(key, entry.key);
        });
    }
    template <typename Pred>
    IteratorType find(unsigned hash, Pred predicate) {
        return table_.find(hash, predicate);
    }

    bool contains(const K& key) const { return find(key) != end(); }

    bool remove(const K& key) {
        auto it = find(key);
        if (it != end()) {
            table_.remove(it);
            return true;
        }

        return false;
    }

    /* TODO: overload insert() to take rvalue of V and move it to
     * table_.insert().
     * This can be done only after HashTable implements forwarding references*/
    HashTableResult insert(const K& key, const V& value) {
        return table_.insert({key, value});
    }
};
}  // namespace hashfu