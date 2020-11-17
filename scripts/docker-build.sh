source /opt/rh/llvm-toolset-7.0/enable
clang -v

export ESY_PREFIX=/esy/store
esy install --fetch-concurrency=1 --build-concurrency=1
esy build --fetch-concurrency=1 --build-concurrency=1
esy run-script bootstrap
node install-node-deps.js --production
esy x  Oni2 -f --checkhealth

esy @test install --fetch-concurrency=1 --build-concurrency=1
esy @test build --fetch-concurrency=1 --build-concurrency=1
esy @test run-script run
esy @test run-script inline

esy @release install --fetch-concurrency=1 --build-concurrency=1 
esy @release build --fetch-concurrency=1 --build-concurrency=1 
esy @release x Oni2 -f --checkhealth
esy @release run-script create
