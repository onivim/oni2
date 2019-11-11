esy install
esy bootstrap
cd node 
node install.js --production
cd ..
esy build
esy x Oni2 -f --checkhealth
esy create-release
