#pragma once

#include <cstddef>

namespace hashfu {

template <typename T, typename TraitsForT>
class HashTable {
    struct Bucket {
        bool used;
        bool deleted;
        T storage;
    };

   private:
    Bucket *buckets_{nullptr};
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

    const Bucket *lookup_for_reading(const T &value) {
        if (is_empty()) return nullptr;

        auto hash = TraitsForT::hash(value);
        auto bucket_index = hash % capacity_;
        auto &bucket = buckets_[bucket_index];

        while (true) {
            if (bucket.used && TraitsForT::equals(bucket, value))
                return &bucket;

            if (!bucket.used && !bucket.deleted) return nullptr;

            // We use linear probing here
            bucket_index++;
            bucket_index = bucket_index % capacity_;
        }
    }

   public:
    HashTable() = default;
};
}  // namespace hashfu