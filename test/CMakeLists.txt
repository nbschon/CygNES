# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(CygNESTests LANGUAGES CXX)

# ---- Tests ----

add_executable(CygNES_test source/CygNES_test.cpp)
target_link_libraries(CygNES_test PRIVATE CygNES_lib)
target_compile_features(CygNES_test PRIVATE cxx_std_17)

add_test(NAME CygNES_test COMMAND CygNES_test)

# ---- End-of-file commands ----

add_folders(Test)
