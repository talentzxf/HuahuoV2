export SCRIPT_DIR=$( dirname $(realpath -s $0) )
rm -rf $SCRIPT_DIR/emcmake
EMCCPATH=`which emcc`
EMCCDIR=`dirname ${EMCCPATH}`
WEBIDL=${EMCCDIR}/tools/webidl_binder
${WEBIDL} ../WebIDL/HuaHuoEngine.idl glue ./emcmake/
emcmake cmake .. && cmake --build ./
cat ./glue.js >> HuaHuoEngine.js