@echo off

FOR %%i IN ("dsaparam.pem" "dsa_priv.pem" "dsa_pub.pem") DO (
  if exist %%i (
    echo There's already a %%i here! Move it aside or be more careful!
    exit /b 1
  )
)

openssl dsaparam 4096 > dsaparam.pem

openssl gendsa -out dsa_priv.pem dsaparam.pem
del /F /Q dsaparam.pem
openssl dsa -in dsa_priv.pem -pubout -out dsa_pub.pem

FOR %%i IN ("dsa_priv.pem" "dsa_pub.pem") DO (
  if not exist %%i (
    echo Failed to create %%i!
    exit /b 1
  )
)

echo[
echo Generated two files:
echo dsa_priv.pem: your private key. Keep it secret and don't share it!
echo dsa_pub.pem: public counterpart to include in youe app.

echo BACK UP YOUR PRIVATE KEY AND KEEP IT SAFE!
echo If you lose it, your users will be unable to upgrade!
echo[
