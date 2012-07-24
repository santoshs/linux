#!/bin/bash

git_clean=`git status | grep "nothing to commit"`

if [ -z "${git_clean}" ]; then
    echo "DIRTY!"
    echo "DIRTY!" > build.id
    exit -1
fi

tmpdir=`mktemp -d /tmp/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX`
trap "rm -Rf ${tmpdir}" TERM ERR INT EXIT
commit=`git log | head -1 | sed -e 's/commit\s//'`

git clone . ${tmpdir}

pushd ${tmpdir}

rm -Rf .git*

timestamp=`date -u +%Y%m%d%H%MZ`

touch release_id.${timestamp}.${commit}

echo "${timestamp} (SHA1: ${commit})" > build.id

zip -m -r infra-signer-build-system.zip *

popd

mv ${tmpdir}/infra-signer-build-system.zip .

unzip -v infra-signer-build-system.zip | grep release_id
