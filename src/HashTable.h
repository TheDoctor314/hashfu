#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>

namespace hashfu {

enum class HashTableResult { InsertedNewEntry, ReplacedExistingEntry };

template <typename HashTableType, typename T, typename Bucket>
class HashTableIterator {
    friend HashTableType;

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

        if (bucket_->end) bucket_ = nullptr;
    }

    explicit HashTableIterator(Bucket* bucket) : bucket_(bucket) {}
};

template <typename T, typename TraitsForT>
class HashTable {
    static constexpr size_t load_factor_percent = 60;

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
    size_t used_buckets_count() const { return size_ + deleted_count_; }
    bool should_grow() const {
        return ((used_buckets_count() + 1) * 100) >=
               (capacity_ * load_factor_percent);
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

    template <typename Pred>
    Bucket* lookup_with_hash(unsigned hash, Pred predicate) {
        if (is_empty()) return nullptr;

        auto index = hash % capacity_;
        for (;;) {
            auto& bucket = buckets_[index];

            if (bucket.used && predicate(*bucket.slot())) return &bucket;

            if (!bucket.used && !bucket.deleted) return nullptr;

            // Linear probing
            index = (index + 1) % capacity_;
        }
    }
    const Bucket* lookup_for_reading(const T& value) const {
        lookup_with_hash(TraitsForT::hash(value), [&value](auto& entry) {
            return TraitsForT::equals(entry, value);
        });
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

        for (const auto& i : other) {
            insert(i);
        }
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

    // NOTE: Debug function
    void print() {
        for (size_t i = 0; i < capacity_; ++i) {
            auto& bucket = buckets_[i];
            std::cerr << i << ": " << *bucket.slot();
            std::cerr << " Used: " << bucket.used;
            std::cerr << " Deleted: " << bucket.deleted;
            std::cerr << " End: " << bucket.end << '\n';
        }
    }

    bool is_empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    float load_factor() const {
        return static_cast<float>(used_buckets_count()) /
               static_cast<float>(capacity());
    }

    //  Implement iterator functions
    using Iterator = HashTableIterator<HashTable, T, Bucket>;
    Iterator begin() {
        for (size_t i = 0; i < capacity_; ++i) {
            if (buckets_[i].used) return Iterator(&buckets_[i]);
        }

        return end();
    }
    Iterator end() { return Iterator(nullptr); }

    using ConstIterator =
        HashTableIterator<const HashTable, const T, const Bucket>;
    ConstIterator begin() const {
        for (size_t i = 0; i < capacity_; ++i) {
            if (buckets_[i].used) return ConstIterator(&buckets_[i]);
        }

        return end();
    }
    ConstIterator end() const { return ConstIterator(nullptr); }

    void clear() { *this = HashTable(); }

    /* TODO: Take a forwarding reference for insert and
     * forward it to the constructor of T*/
    HashTableResult insert(const T& value) {
        auto& bucket = lookup_for_writing(value);

        if (bucket.used) {
            *bucket.slot() = value;
            return HashTableResult::ReplacedExistingEntry;
        }

        new (bucket.slot()) T(value);
        bucket.used = true;
        if (bucket.deleted) {
            // if we are reusing a deleted entry, decrease count
            bucket.deleted = false;
            --deleted_count_;
        }

        ++size_;
        return HashTableResult::InsertedNewEntry;
    }
};
}  // namespace hashfu
