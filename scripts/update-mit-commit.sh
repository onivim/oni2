DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
echo "Onivim 2 source repo: $DIR"
echo "Onivim 2-MIT repo: $1"
MIT_COMMIT=$(node $DIR/get-mit-commit.js)
echo "Latest MIT-licensed commit: $MIT_COMMIT"

echo "Snapping repo to commit.."
cd "$1"
pwd

git remote add upstream https://github.com/onivim/oni2
git fetch upstream
git reset --hard $MIT_COMMIT

echo "Generating README from template..."
node "$DIR/generate-mit-readme.js" "$MIT_COMMIT" "$1"
cd "$1"

git add README.md
git commit -m "$MIT_COMMIT"
git push --force origin master
