esy install
esy bootstrap
node install-node-deps.js --production
esy build
esy x Oni2 -f --checkhealth
esy create-release
