var LibraryProfilerWebGL = {

JS_Profiler_InjectJobs__proxy: 'sync',
JS_Profiler_InjectJobs__sig: 'v',
JS_Profiler_InjectJobs: function()
{
  for (var jobname in Module["Jobs"])
  {
    var job = Module["Jobs"][jobname];
    if (typeof job["endtime"] != "undefined")
       Module.ccall("InjectProfilerSample", null, ["string", "number", "number"], [jobname, job.starttime, job.endtime]);
   }
}
};

mergeInto(LibraryManager.library, LibraryProfilerWebGL);
