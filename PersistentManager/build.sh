cd ./emcmake
rm -rf ./*
EMCCPATH=`which emcc`
EMCCDIR=`dirname ${EMCCPATH}`
WEBIDL=${EMCCDIR}/tools/webidl_binder
${WEBIDL} ../WebIDL/PersistentManager.idl glue ./emcmake/
emcmake cmake .. && cmake --build ./
