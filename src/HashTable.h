#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>

namespace hashfu {

enum class HashTableResult { InsertedNewEntry, ReplacedExistingEntry };

template <typename HashTableType, typename T, typename Bucket>
class HashTableIterator {
   private:
    Bucket* bucket_{nullptr};  // points to the current bucket

   public:
    // Basic operator overloads required by any iterator
    bool operator==(const HashTableIterator& rhs) {
        return bucket_ == rhs.bucket_;
    }
    bool operator!=(const HashTableIterator& rhs) {
        return bucket_ != rhs.bucket_;
    }
    T& operator*() { return *bucket_->slot(); }
    T* operator->() { return bucket_->slot(); }

    void operator++() { next_used_bucket(); }

   private:
    void next_used_bucket() {
        if (!bucket_) return;

        do {
            ++bucket_;
            if (bucket_->used) return;
        } while (!bucket_->end);

        if (!bucket_->end) bucket_ = nullptr;
    }
};

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

        T* slot() { return reinterpret_cast<T*>(storage); }
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

        for (size_t i = 0; i < old_capacity; i++) {
            // move from old table to new
            auto& old_bucket = old_buckets[i];

            // We insert only used objects, not deleted objects
            if (old_bucket.used) {
                insert_during_rehash(*old_bucket.slot());
                old_bucket.slot()->~T();
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
    explicit HashTable(size_t capacity) { rehash(capacity); }

    ~HashTable() {
        if (!buckets_) return;

        // iterate through array, destroying each object
        for (size_t i = 0; i < capacity_; ++i) {
            if (buckets_[i].used) {
                buckets_[i].slot()->~T();
            }
        }

        std::free(buckets_);
    }

    // copy constructor
    HashTable(const HashTable& other) {
        rehash(other.capacity());

        /* TODO: iterate through other and set each element.
         * This requires implementing iterators.*/
    }

    // move constructor
    HashTable(HashTable&& other) noexcept
        : size_(other.size_),
          capacity_(other.capacity_),
          buckets_(other.buckets_),
          deleted_count_(other.deleted_count_) {
        other.buckets_ = nullptr;
    }
    // move assignment
    HashTable& operator=(HashTable&& other) noexcept {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(deleted_count_, other.deleted_count_);
        std::swap(buckets_, other.buckets_);
        return *this;
    }
};
}  // namespace hashfu