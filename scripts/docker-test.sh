source /opt/rh/llvm-toolset-7.0/enable
clang -v

echo "Deleting $(ESY_CACHE_BUILD_PATH)"
rm -rf $(ESY__CACHE_BUILD_PATH)

echo "Deleting esy folder"
rm -rf _esy

esy @test install
esy @test build
esy @test run
