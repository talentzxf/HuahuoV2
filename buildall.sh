export SCRIPT_DIR=$( dirname $(realpath -s $0) )

buildProd=false
if [ -n "$1" ] && [ $1 == "prod" ]; then
  buildProd=true
  echo "Building for PROD"
else
  echo "Building for default"
fi

buildComponent(){
  cd $SCRIPT_DIR/$1
  rm -rf ./package-lock.json
  npm install

  if [ $buildProd == "true" ]; then
    echo "npm run buildProd"
    npm run buildProd
  else
    echo "npm run build"
    npm run build
  fi
}

buildComponent HHCommonComponents
buildComponent HHEngineJS
buildComponent HHPanel
buildComponent HHTimeLine
buildComponent HHIDE

