rm -rf ~/Library/Keychains/onivim.keychain*

openssl genrsa -out key.pem 2048
echo "Created key.."
openssl req -new -sha256 -key key.pem -out csr.csr -subj /C=US/ST=Seattle/L=Seattle/O=Onivim/OU=OpenSource/CN=local.outrunlabs.com -config scripts/osx/temporary-cert.conf -reqexts v3_req
echo "Created request"
openssl req -x509 -new -config scripts/osx/temporary-cert.conf -extensions v3_req -sha256 -out certificate.crt -key key.pem
echo "Exporting"
openssl pkcs12 -export -inkey key.pem -in certificate.crt -out certificate.pem
#openssl x509 -req -sha256 -days 10 -signkey key.pem -in csr.csr -out certificate.pem
certtool i certificate.pem k="onivim.keychain" r=./key.pem c p=moof
security add-trusted-cert -d -r trustRoot -k onivim.keychain  certificate.pem
echo "Creating cert"

#openssl pkcs12 -export -out certificate.p12 -inkey key.pem -in certificate.pem -password pass:password!
