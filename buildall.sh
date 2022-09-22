export SCRIPT_DIR=$( dirname $(realpath -s $0) )

buildComponent(){
  cd $SCRIPT_DIR/$1
  npm install

  if [ -n "$1" ]
    npm run buildProd
  else
    npm run build
  fi
}

buildComponent HHCommonComponents
buildComponent HHEngineJS
buildComponent HHPanel
buildComponent HHTimeLine
buildComponent HHIDE

