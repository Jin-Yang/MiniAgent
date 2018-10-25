#!/bin/sh
# usage: git-version-gen [srcdir]
# Get the version in following orders:
#   1. version file, generally included in release tarballs.
#   2. git describe.
#   3. default value.

DEFAULT_VERSION="v0.1.0"

LF='
'

SRCDIR="."
if test "x${1}" != "x"; then
	SRCDIR="${1}"
fi

cd "${SRCDIR}" || exit 1

if [ -f VERSION ]; then
	VN="`cat VERSION`" || VN="${DEFAULT_VERSION}"
elif [ -d .git ]; then
	VN="`git describe --match "v[0-9]*" --dirty=+ --abbrev=7 2>/dev/null `"
fi

if [ -z "${VN}" ]; then
	VN="${DEFAULT_VERSION}"
fi

VN="`expr "${VN}" : v*'\(.*\)'`"

echo "${VN}" | tr -d "$LF"

