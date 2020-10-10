source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy install
esy bootstrap
esy build

esy @test install
esy @test build
esy @test run
esy @test inline
