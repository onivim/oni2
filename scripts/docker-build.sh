source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy install --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy build --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy run-script --prefix-path=/esy/store bootstrap
node install-node-deps.js --production
esy x --prefix-path=/esy/store Oni2 -f --checkhealth

esy @test install --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy @test build --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy @test run-script --prefix-path=/esy/store run
esy @test run-script --prefix-path=/esy/store inline

esy @release install --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy @release build --fetch-concurrency=1 --build-concurrency=1 --prefix-path=/esy/store
esy @release x --prefix-path=/esy/store Oni2 -f --checkhealth
esy @release run-script --prefix-path=/esy/store create
