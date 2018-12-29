#!/bin/bash

set -o nounset
set -o errexit

usage() {
        echo "USAGE: $0 <PROJECT_NAME> <VERSION>"
        echo " e.g.: $0 BootAgent 1.3.2-1"
}

if [ $# -lt 2 ]; then
        usage
        exit 1;
fi

if [ ! -f "contrib/generate_version.sh" ]; then
        echo "ERROR: this script MUST run on the top of source code"
        exit 1;
fi

command -v rpmbuild >/dev/null 2>&1 || { echo "ERROR: command 'rpmbuild' doesn't exist"; exit 1; }

PACKAGE_NAME=${1}
PROJECT_VERSION=${2}
GENER_VERSION="`contrib/generate_version.sh`"
RPM_DIR="`pwd`/rpm-maker"
SPEC_FILE="`pwd`/contrib/${PACKAGE_NAME}.spec"
PKG_VERSION=${PROJECT_VERSION%-*}
PKG_RELEASE=${PROJECT_VERSION#*-}
if [ ${PKG_RELEASE} == ${PKG_VERSION} ]; then
	PKG_RELEASE='0'
fi
PACKAGE_TAR="${PACKAGE_NAME}-${PKG_VERSION}-${PKG_RELEASE}.tar.bz2"
PKG_COMMITID=` git rev-parse HEAD`

if [ x${PKG_VERSION} != x${GENER_VERSION} ]; then
        echo "ERROR: unequal version, generate version ${PROJECT_VERSION} != ${GENER_VERSION}."
        exit 1;
fi

echo "Package RPM, Version(${PKG_VERSION}) Release(${PKG_RELEASE})."
echo "++++++++++++++++++++++++++ step 0 ++++++++++++++++++++++++++++++"
echo "Generate source archive '${PACKAGE_TAR}'."
if [ -f "${PACKAGE_TAR}" ]; then
	echo "Clean up ${PACKAGE_TAR}."
        rm -f "${PACKAGE_TAR}"
fi
echo "Create ${PACKAGE_TAR} package."
tar --transform "s%^%${PACKAGE_NAME}-${PKG_VERSION}-${PKG_RELEASE}/%" -jcf ${PACKAGE_TAR} \
        daemon                                                                            \
        libs                                                                              \
        include                                                                           \
        contrib                                                                           \
        CMakeLists.txt                                                                    \
        README.md

echo "++++++++++++++++++++++++++ step 1 ++++++++++++++++++++++++++++++"
echo "Create RPM workspace at '${RPM_DIR}'"
if [ ! -d "${RPM_DIR}" ]; then
        mkdir -p "${RPM_DIR}"
else
        rm -rf "${RPM_DIR}"
fi
mkdir -p "${RPM_DIR}"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
cp "${SPEC_FILE}" "${RPM_DIR}"/SPECS/${PACKAGE_NAME}.spec
sed -i "s/^%define package_version .*$/%define package_version ${PROJECT_VERSION}/g" "${RPM_DIR}"/SPECS/${PACKAGE_NAME}.spec
mv "${PACKAGE_TAR}" "${RPM_DIR}"/SOURCES/

echo "++++++++++++++++++++++++++ step 2 ++++++++++++++++++++++++++++++"
# ignore debuginfo ===> --define "debug_package %{nil}"                     \
rpmbuild --define "_topdir ${RPM_DIR}"                       \
         --define "pkg_version ${PKG_VERSION}"               \
         --define "pkg_release ${PKG_RELEASE}"               \
         --define "commitid ${PKG_COMMITID}"                 \
	 -ba "${RPM_DIR}"/SPECS/${PACKAGE_NAME}.spec

