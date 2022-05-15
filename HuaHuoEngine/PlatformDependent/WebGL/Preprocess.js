var locals = {
  fs: require("fs"),
  path: require("path"),
  assert: require("assert"),
  evaluate: (string) => eval(string),
};

function readBinary(filename) {
  var relativePath = locals.path.join(locals.path.dirname(locals.inputPath), filename);
  return locals.fs.readFileSync(locals.fs.existsSync(relativePath) ? relativePath : filename);
}

function read(filename) {
  return readBinary(filename).toString();
}

for (locals.i = 2; locals.i < process.argv.length; locals.i++) {
  locals.assert(/^(locals\.(inputPath|outputPath|minifyOutput)|[a-zA-Z_$][a-zA-Z0-9_$]*)=(true|false|\d+|'[0-9a-zA-Z \.\/\\]*')$/.test(process.argv[locals.i]), "Invalid preprocessor argument: " + process.argv[locals.i]);
  eval(process.argv[locals.i]);
}

if (!locals.outputPath) {
  locals.variables = {};
  locals.parseGlobals = require(locals.path.resolve("acorn/node_modules/acorn-globals"));
  locals.evaluate = (string) => {
    locals.parseGlobals(string).forEach(function (global) {
      if (!locals.variables[global.name] && /^[a-zA-Z_$][a-zA-Z0-9_$]*$/.test(global.name) && eval('typeof ' + global.name + ' == "undefined"'))
        locals.variables[global.name] = true;
    });
    return true;
  };
}

locals.output = "";
locals.showStack = [];
locals.fs.readFileSync(locals.inputPath, "utf8").replace(/\n$/, "").split("\n").forEach(function (line, index) {
  try {
    if (!line.match(/^\s*#(if|else|endif)(\s|$)/)) {
      if (locals.showStack.indexOf(false) == -1)
        locals.output += line + "\n";
    } else if (line.match(/^\s*#if\s+[^\s]/)) {
      locals.showStack.push(!!locals.evaluate(line.substring(line.indexOf("#") + 3)));
    } else if (line.match(/^\s*#else(\s*$|\s+\/\/)/)) {
      locals.assert(locals.showStack.length, "found #else without matching #if");
      locals.showStack.push(!locals.showStack.pop() || !locals.outputPath);
    } else if (line.match(/^\s*#endif(\s*$|\s+\/\/)/)) {
      locals.assert(locals.showStack.length, "found #endif without matching #if");
      locals.showStack.pop();
    } else {
      throw "invalid preprocessor command"
    }
  } catch (e) {
    throw "Preprocessor error \"" + e + "\" occured in file \"" + locals.inputPath + "\" at line " + (index + 1) + " when evaluating expression \"" + line + "\"";
  }
});
locals.assert(!locals.showStack.length, "Preprocessor error \"missing #endif\" occured in file \"" + locals.inputPath + "\"");
locals.output = locals.output.replace(/{{{([^}]|}(?!}))+}}}/g, function (preprocessedBlock) {
  var expression = preprocessedBlock.substring(3, preprocessedBlock.length - 3);
  try {
    var value = locals.evaluate(expression);
    return value !== null ? value.toString() : "";
  } catch (e) {
    throw "Preprocessor error \"" + e + "\" occured in file \"" + locals.inputPath + "\" when evaluating expression \"" + expression + "\"";
  }
});

if (!locals.outputPath) {
  console.log(JSON.stringify(Object.keys(locals.variables)));
  return;
}

if (locals.minifyOutput)
  locals.output = require(locals.path.resolve("uglify-js")).minify(locals.output, { fromString: true, output: { "ascii_only": true } }).code;
locals.fs.writeFileSync(locals.outputPath, locals.output);
