export SCRIPT_DIR=$( dirname $(realpath -s $0) )

export EMCMAKE_DIR=$SCRIPT_DIR/emcmake
echo "Deleting temp dir:$EMCMAKE_DIR"
rm -rf $EMCMAKE_DIR
mkdir $EMCMAKE_DIR
echo "cd $EMCMAKE_DIR"
cd $EMCMAKE_DIR
EMCCPATH=`which emcc`
EMCCDIR=`dirname ${EMCCPATH}`
WEBIDL=${EMCCDIR}/tools/webidl_binder

echo > ./MergedIDL.idl
cat ../WebIDL/HuaHuoEngine.idl  >>  ./MergedIDL.idl
#cat ../WebIDL/HuaHuoEditor.idl >> ./MergedIDL.idl
#cat ../WebIDL/HuaHuoGraphics.idl >> ./MergedIDL.idl

${WEBIDL} ./MergedIDL.idl glue ./emcmake/

if [ $? -eq 0 ]; then
   emcmake cmake .. && cmake --build ./
   cat ./glue.js >> HuaHuoEngineV2.js
else
   echo "Failed to run webidl_binder"
fi
