add_subdirectory(iop)

if(USE_AI)
  add_subdirectory(ai)
endif(USE_AI)

add_cmocka_test(test_sample
                SOURCES test_sample.c
                LINK_LIBRARIES cmocka)
