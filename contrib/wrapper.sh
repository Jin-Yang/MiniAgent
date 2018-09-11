#! /bin/sh
#
# A wrapper script for running tests. If valgrind is available, memory
# checking will be enabled.

set -e

MEMCHECK=""

if hash valgrind 2>/dev/null; then
        echo "the VALGRIND is turn on"
        MEMCHECK="valgrind --quiet --tool=memcheck --error-exitcode=1"
        MEMCHECK="$MEMCHECK --leak-check=full"
        MEMCHECK="$MEMCHECK --trace-children=yes"
        MEMCHECK="$MEMCHECK --show-leak-kinds=all"
        MEMCHECK="$MEMCHECK --show-reachable=yes"
        MEMCHECK="$MEMCHECK --gen-suppressions=all"
        #MEMCHECK="$MEMCHECK --log-file=valgrind.log"

        for f in "valgrind.$( uname -s ).suppress" "valgrind.suppress"; do
                filename="$( dirname "$0" )/src/$f"
                if test -e "$filename"; then
                        # Valgrind supports up to 100 suppression files.
                        MEMCHECK="$MEMCHECK --suppressions=$filename"
                fi
        done
else
        echo "valgrind not exist"
fi

exec $MEMCHECK "$@"
