set -e

echo "The script you are running has basename `basename "$0"`, dirname `dirname "$0"`"
echo "The present working directory is `pwd`"

CWD=`dirname $0`
echo "cur__bin: $cur__bin"
echo "wd: $CWD"
cd $CWD
echo "wd: $CWD"
ls
echo "!!! Starting integration tests"
for file in ./*Test.exe;
do
      echo "-- Running test $file"
      "$file"
      echo "-- Test complete"
done
echo "!!! integration tests complete!"
