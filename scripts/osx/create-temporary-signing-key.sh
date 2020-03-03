openssl genrsa -out key.pem 2048
echo "Created key.."
openssl req -new -sha256 -key key.pem -out csr.csr -subj /C=US/ST=Seattle/L=Seattle/O=Onivim/OU=OpenSource/CN=local.outrunlabs.com 
echo "Created request"
openssl x509 -req -sha256 -days 10 -signkey key.pem -in csr.csr -out certificate.pem -extensions codesign
certtool i certificate.pem k="key.keychain" r=./key.pem c p=moof
echo "Creating cert"
#openssl pkcs12 -export -out certificate.p12 -inkey key.pem -in certificate.pem -password pass:password!
