source /opt/rh/llvm-toolset-7.0/enable
clang -v

# Use build cache to speed up PR builds
export ESY__PREFIX=/esy/store

# Workaround for: https://github.com/esy/esy/issues/1227
# Concurrent fetch seems to cause hang on Docker in Azure Pipelines..
export ESY__BUILD_CONCURRENCY=1
export ESY__FETCH_CONCURRENCY=1

esy install 
esy build 
node install-node-deps.js --production
esy run-script bootstrap

esy @test install 
esy @test build 
esy @test run-script run
esy @test run-script inline

esy @release install 
esy @release build 
esy @release x Oni2 --version
esy @release run-script create
