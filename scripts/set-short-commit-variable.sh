cd oni2


SHORTCOMMITID=$(git rev-parse --short HEAD)


echo "##vso[task.setvariable variable=ONI2_SHORT_COMMIT_ID;isOutput=true]$SHORTCOMMITID"


