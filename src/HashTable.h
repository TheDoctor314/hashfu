#pragma once

#include <cstddef>

namespace hashfu {

template <typename T, typename TraitsForT>
class HashTable {
    struct Bucket {
        bool used;
        bool deleted;
        T storage;

        T* slot() { return &storage; }
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

    void rehash(size_t new_capacity);

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