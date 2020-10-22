cd src

echo "Linting in : $(pwd)"

PRERR_COUNT=$(git grep prerr_endline | wc -l)
PRINT_COUNT=$(git grep print_endline | wc -l)
PRINTF_COUNT=$(git grep Printf.printf | wc -l)

echo "prrerr_endline count: $PRERR_COUNT"
echo "print_endline count: $PRINT_COUNT"
echo "Printf.printf count: $PRINTF_COUNT"

if [ "$PRERR_COUNT" -ne "7" ]; then
    echo "New prerr_endline introduced; please remove and re-check:"
    echo "---"
    echo "Output: $(git grep prerr_endline)"
    exit 3
fi

if [ "$PRINT_COUNT" -ne "16" ]; then
    echo "New print_endline introduced; please remove and re-check."
    echo "---"
    echo "Output: $(git grep print_endline)"
    exit 3
fi

if [ "$PRINTF_COUNT" -ne "3" ]; then
    echo "New Printf.printf introduced; please remove and re-check."
    echo "---"
    echo "Output: $(git grep Printf.printf)"
    exit 3
fi

echo "Lint check OK"
