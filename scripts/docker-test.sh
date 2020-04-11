source /opt/rh/llvm-toolset-7.0/enable
clang -v

echo "Deleting esy folder"
rm -rf _esy

esy cleanup test.json --dry-run
esy cleanup test.json

esy @test install
esy @test build
esy @test run
