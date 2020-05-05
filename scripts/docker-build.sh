source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy install
esy bootstrap
node install-node-deps.js --production
esy build
esy x Oni2 -f --checkhealth
esy create-release

esy @test install
esy @test build
esy @test run
