hashtable_test_sources = files(
  'catch_main.cpp',
  'hashtable_tests.cpp',
  )

hashmap_test_sources = files(
  'catch_main.cpp',
  'hashmap_tests.cpp',
  )

hashtable_test = executable('hashtable_test', hashtable_test_sources, include_directories: hashfu_inc)
hashmap_test = executable('hashmap_test', hashmap_test_sources, include_directories: hashfu_inc)

test('HashTable', hashtable_test)
test('HashMap', hashmap_test)
