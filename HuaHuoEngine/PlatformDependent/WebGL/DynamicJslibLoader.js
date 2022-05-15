LibraryManager = {};
function emscriptenFinalName(ident) {
  var finalName = '_' + ident;
  if (ident[0] === '$') {
    finalName = ident.substr(1);
  }
  return finalName;
}      
function mergeInto(obj, other) {
  for (var name in other) {
    Module[emscriptenFinalName(name)] = other[name];
  }
}
function autoAddDeps(object, name) {
  Module[emscriptenFinalName(name)] = object[name];
}
