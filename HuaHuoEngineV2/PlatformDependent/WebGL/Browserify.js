const fs = require("fs");
const path = require("path");

var options = {
  "--output": "",
  "--uglify": "",
  "--root": "",
  "--module": [],
};
for (var i = 2; i < process.argv.length;) {
  var option = process.argv[i++];
  if (typeof options[option] == "string") {
    options[option] = process.argv[i++];
  } else if (typeof options[option] == "object") {
    options[option].push(process.argv[i++]);
  } else {
    throw "invalid option: " + option;
  }
}
for (var option in options) {
  if (!options[option].length)
    throw "missing option: " + option;
}

const Framework = {
  resolveLocal: function (path) {
    var parts = [];
    path = path.split("/").every(function (p) { return p == ".." ? parts.pop() : p == "." || p == "" ? true : parts.push(p); }) ? parts.join("/") : null;
    return path ? modules[path] || modules[path + ".js"] || modules[path + "/index.ts"] : null;
  },
  resolveGlobal: function (module, path) {
    return module ? resolveLocal(module.folder + "node_modules/" + path) || resolveGlobal(module.parent, path) : null;
  },
  require: function (parent, path) {
    var module = path.match(/^\//) ? null : !parent ? resolveLocal(path) : path.match(/^\.\.?\//) ? resolveLocal(parent.folder + path) : resolveGlobal(parent, path);
    if (!module)
      throw "module not found: " + path;
    if (!module.exports) {
      module.parent = parent;
      module(require.bind(null, module), module, module.exports = {});
    }
    return module.exports;
  },
};

var result = "function require(path) {\nvar modules = {\n";
options["--module"].forEach(function (module) { result += "\"" + module + "\": function (require, module, exports) {\n" + fs.readFileSync(path.join(options["--root"], module), "utf8") + "\n},\n"; });
result += "};\nfor (var module in modules)\n  modules[module].folder = module.substring(0, module.lastIndexOf(\"/\") + 1);";
for (var symbol in Framework)
  result += "\nvar " + symbol + " = " + Framework[symbol].toString().replace(/\n  /g, "\n") + ";";
result += "\nreturn require(null, path);\n}";
fs.writeFileSync(options["--output"], require(options["--uglify"]).minify(result, { fromString: true, output: { "ascii_only": true } }).code);
