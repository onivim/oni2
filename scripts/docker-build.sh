source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy install --fetch-concurrency=1 --build-concurrency=1 -vv
esy build --fetch-concurrency=1 --build-concurrency=1 -vv
esy bootstrap --fetch-concurrency=1 --build-concurrency=1 -vv
node install-node-deps.js --production
esy x Oni2 -f --checkhealth

# esy @release install
# esy @release build
# esy @release x Oni2 -f --checkhealth
# esy @release create
