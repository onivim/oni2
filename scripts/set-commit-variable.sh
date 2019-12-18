COMMITID=$(git log -n 1 --pretty=format:"%H")

echo "##vso[task.setvariable variable=ONI2_COMMIT_ID;isOutput=true]$COMMITID"


