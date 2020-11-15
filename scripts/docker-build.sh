source /opt/rh/llvm-toolset-7.0/enable
clang -v

ESY__BUILD_CONCURRENCY=1
ESY__FETCH_CONCURRENCY=1
esy install -vv
esy build -vv
esy bootstrap -vv
node install-node-deps.js --production
esy x Oni2 -f --checkhealth

esy @release install
esy @release build
esy @release x Oni2 -f --checkhealth
esy @release create
