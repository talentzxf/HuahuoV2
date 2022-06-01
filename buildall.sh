export SCRIPT_DIR=$( dirname $(realpath -s $0) )

buildComponent(){
  cd $SCRIPT_DIR/$1
  npm install
  npm run build
}

buildComponent HHCommonComponents
buildComponent HHPanel
buildComponent HHTimeLine
buildComponent HHEngineJS
buildComponent HHIDE

