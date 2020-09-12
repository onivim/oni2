source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy @release install
esy bootstrap
node install-node-deps.js --production
esy @release build
esy @release x Oni2 -f --checkhealth
esy @release create
