#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>

namespace hashfu {

template <typename T, typename TraitsForT>
class HashTable {
    struct Bucket {
        bool used;
        bool deleted;
        bool end;

        /*This trick allows us to construct object of type T in this memory
         *space. To construct an object in this space, use the placement new
         *syntax. https://isocpp.org/wiki/faq/dtors#placement-new
         */
        alignas(T) unsigned char storage[sizeof(T)];

        T* slot() { return static_cast<T*>(storage); }
    };

   private:
    Bucket* buckets_{nullptr};
    size_t size_{0};
    size_t deleted_count_{0};
    size_t capacity_{0};

   private:
    bool is_empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    float load_factor() const {
        return static_cast<float>(size()) / static_cast<float>(capacity());
    }

    size_t used_buckets_count() const { return size_ + deleted_count_; }
    bool should_grow() const {
        if (used_buckets_count() >= capacity()) {
            return true;
        }
        return false;
    }

    void rehash(size_t new_capacity) {
        new_capacity = std::max(new_capacity, static_cast<size_t>(4));

        auto old_capacity = capacity_;
        auto* old_buckets = buckets_;

        auto* new_buckets =
            (Bucket*)std::malloc(sizeof(Bucket) * (new_capacity + 1));
        if (!new_buckets) {
            return;
        }
        __builtin_memset(new_buckets, 0, sizeof(Bucket) * (new_capacity + 1));

        buckets_ = new_buckets;
        capacity_ = new_capacity;
        deleted_count_ = 0;

        // sentinel pointer to mark end of line
        buckets_[capacity_].end = true;

        if (!old_buckets) return;

        for (int i = 0; i < old_capacity; i++) {
            // move from old table to new
            auto& old_bucket = old_buckets[i];

            // We insert only used objects, not deleted objects
            if (old_bucket.used) {
                insert_during_rehash(*old_bucket.slot());
                old_bucket.slot->~T();
            }
        }

        std::free(old_buckets);
    }

    void insert_during_rehash(T& val) {
        auto& bucket = lookup_for_writing(val);

        /* We use the placement new syntax. This allows us to
         * provide the address where the object should be constructed.*/
        new (bucket.slot()) T(val);
        bucket.used = true;
    }

    const Bucket* lookup_for_reading(const T& value) {
        if (is_empty()) return nullptr;

        auto hash = TraitsForT::hash(value);
        auto bucket_index = hash % capacity_;
        auto& bucket = buckets_[bucket_index];

        while (true) {
            if (bucket.used && TraitsForT::equals(*bucket.slot(), value))
                return &bucket;

            if (!bucket.used && !bucket.deleted) return nullptr;

            // We use linear probing here
            bucket_index++;
            bucket_index = bucket_index % capacity_;
        }
    }

    Bucket& lookup_for_writing(const T& value) {
        if (should_grow()) {
            rehash(capacity() * 2);
        }

        auto hash = TraitsForT::hash(value);
        auto bucket_index = hash % capacity_;

        /*We need to return the first empty bucket we come across or the bucket
        which needs to be replaced*/
        Bucket* first_empty_bucket = nullptr;
        while (true) {
            auto& bucket = buckets_[bucket_index];

            if (bucket.used && TraitsForT::equals(*bucket.slot(), value))
                return bucket;

            if (!bucket.used) {
                if (!first_empty_bucket) first_empty_bucket = &bucket;

                if (!bucket.deleted) return *first_empty_bucket;
            }

            bucket_index = (bucket_index + 1) % capacity_;
        }
    }

   public:
    HashTable() = default;
};
}  // namespace hashfu