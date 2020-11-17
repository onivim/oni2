source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy install --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy build --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy bootstrap --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
node install-node-deps.js --production
esy x Oni2 -f --checkhealth

esy @test install --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy @test build --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy @test run --prefix-path=/esy/store
esy @test inline --prefix-path=/esy/store

esy @release install --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy @release build --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy @release x Oni2 -f --checkhealth
esy @release create
