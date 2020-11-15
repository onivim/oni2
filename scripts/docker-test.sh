source /opt/rh/llvm-toolset-7.0/enable
clang -v

ESY__BUILD_CONCURRENCY=1
ESY__FETCH_CONCURRENCY=1

esy install
esy bootstrap
esy build

esy @test install
esy @test build
esy @test run
esy @test inline
