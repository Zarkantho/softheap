#include <greatest.h>
#include "store.h"

// This is duplicated to avoid externing this struct
struct mmap_store {
    store_t store;
    int fd;
    int flags;
    uint64_t capacity;
    uint64_t sync_cursor;
    uint64_t write_cursor;
    void* mapping;
};

static struct mmap_store *store;
static const uint64_t SIZE = 1024 * 1024 * 64;

TEST test_size_written() {
    // Break encapsulation (naughty naughty)
    uint64_t *store_as_ints = (uint64_t *) store->mapping;
    ASSERT_EQ(0xDEADBEEF, store_as_ints[0]);
    ASSERT_EQ(SIZE, store_as_ints[1]);
    PASS();
}

TEST test_basic_store() {
    size_t size = 250 * sizeof(char);
    char *data = (char*) calloc(1, size);
    ASSERT(data != NULL);

    uint64_t curr_offset = ((store_t*)store)->cursor(store);
    ASSERT(curr_offset == sizeof(uint64_t) * 2);

    uint64_t offset = ((store_t*)store)->write(store, data, size);
    ASSERT(offset > 0);

    ASSERT_EQ(curr_offset, offset);

    uint64_t new_offset = ((store_t*)store)->cursor(store);
    // There is one uint64 at the start of the store
    // anything else is the offset + stuff
    ASSERT_EQ(size + sizeof(uint64_t) + curr_offset, new_offset);
    PASS();
}

SUITE(mmap_store_suite) {
    RUN_TEST(test_size_written);
    RUN_TEST(test_basic_store);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();
    store = (struct mmap_store*) create_mmap_store(SIZE, ".", "test_store.str", 0);
    ASSERT(store != NULL);

    RUN_SUITE(mmap_store_suite);
    GREATEST_MAIN_END();
}
