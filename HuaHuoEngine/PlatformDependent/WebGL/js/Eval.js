var LibraryEvalWebGL = {

JS_Eval_EvalJS__proxy: 'sync',
JS_Eval_EvalJS__sig: 'vi',
JS_Eval_EvalJS: function(ptr)
{
	var str = Pointer_stringify(ptr); // TODO: Replace all uses of Pointer_stringify() with UTF8ToString(), AsciiToString(), which is a better function
	try {
		eval (str);
	}
	catch (exception)
	{
		console.error(exception);
	}
},

JS_Eval_OpenURL__proxy: 'sync',
JS_Eval_OpenURL__sig: 'vi',
JS_Eval_OpenURL: function(ptr)
{
	var str = Pointer_stringify(ptr);
	window.open(str, '_blank', '');
},

JS_Eval_SetInterval__proxy: 'sync',
JS_Eval_SetInterval__sig: 'iiii',
JS_Eval_SetInterval: function(func, arg, millis)
{
    Module['noExitRuntime'] = true;

    function wrapper() {
      getFuncWrapper(func, 'vi')(arg);
    }

	return Browser.safeSetInterval(wrapper, millis);
},

JS_Eval_ClearInterval__proxy: 'sync',
JS_Eval_ClearInterval__sig: 'vi',
JS_Eval_ClearInterval: function(id)
{
	window.clearInterval(id);
},

JS_Eval_SetTimeout__proxy: 'sync',
JS_Eval_SetTimeout__sig: 'iiii',
JS_Eval_SetTimeout: function(func, arg, millis)
{
    Module['noExitRuntime'] = true;

    function wrapper() {
      getFuncWrapper(func, 'vi')(arg);
    }

	return Browser.safeSetTimeout(wrapper, millis);
},

JS_Eval_ClearTimeout__proxy: 'sync',
JS_Eval_ClearTimeout__sig: 'vi',
JS_Eval_ClearTimeout: function(id)
{
	window.clearTimeout(id);
}


};

mergeInto(LibraryManager.library, LibraryEvalWebGL);
