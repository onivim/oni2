source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy @test install
esy @test build
esy @test run
esy @test inline
