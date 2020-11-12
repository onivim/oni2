source /opt/rh/llvm-toolset-7.0/enable
clang -v

esy install
esy build
esy bootstrap
node install-node-deps.js --production
esy x Oni2 -f --checkhealth

esy @release install
esy @release build
esy @release x Oni2 -f --checkhealth
esy @release create
