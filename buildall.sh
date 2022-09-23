export SCRIPT_DIR=$( dirname $(realpath -s $0) )

buildProd=false
if [ ! -z "$1" ] and [ $1 == "prod" ]; then
  buildProd=true
fi

buildComponent(){
  cd $SCRIPT_DIR/$1
  npm install

  if [ buildProd ] && [ $1 == "HHIDE" ]; then
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

