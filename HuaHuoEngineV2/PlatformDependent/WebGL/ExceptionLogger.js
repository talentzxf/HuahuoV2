var ___cxa_throw = (function() {
    var original___cxa_throw = ___cxa_throw;

    return function() {
        console.log("Exception at: \n"+stackTrace());
        original___cxa_throw();
    }
})();