source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy install --fetch-concurrency=1 --build-concurrency=1 -vv
esy build --fetch-concurrency=1 --build-concurrency=1 -vv
esy bootstrap --fetch-concurrency=1 --build-concurrency=1 -vv
node install-node-deps.js --production
esy x Oni2 -f --checkhealth

esy @test install --fetch-concurrency=1 --build-concurrency=1
esy @test build --fetch-concurrency=1 --build-concurrency=1
esy @test run
esy @test inline

esy @release install --fetch-concurrency=1 --build-concurrency=1
esy @release build --fetch-concurrency=1 --build-concurrency=1
esy @release x Oni2 -f --checkhealth
esy @release create
