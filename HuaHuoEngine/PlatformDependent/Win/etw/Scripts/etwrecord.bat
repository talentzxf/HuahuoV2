@setlocal
cd /D %~dp0\..
@rem Add this batch file's directory to the path.
@set batchdir=%~dp0
@set path=%path%;%batchdir%

@call etwcommonsettings.bat

@if "%1" == "" goto NoFileSpecified
@set FileName=%1
@goto FileSpecified

:NoFileSpecified
@rem Generate a file name based on the current date and time and put it in
@rem the parent directory of the batch file.
@mkdir Captures
@for /F "tokens=2-4 delims=/- " %%A in ('date/T') do @set datevar=%%C-%%A-%%B
@for /F "tokens=1-3 delims=:- " %%A in ('time/t') do @set timevar=%%A-%%B-%%C
@set FileName=%batchdir%Captures\%username%_%datevar%-%timevar%-trace.etl
@echo No filename specified. Trace will be saved to %FileName%

:FileSpecified
@rem Set trace parameters. Latency is a good default group, and Power adds
@rem CPU power management details. Dispatcher allows wait classification and
@rem file IO is useful for seeing what disk reads are requested.
@rem Note that Latency is equal to PROC_THREAD+LOADER+DISK_IO+HARD_FAULTS+DPC+INTERRUPT+CSWITCH+PROFILE
@set KernelProviders=Latency+POWER+DISPATCHER+FILE_IO+FILE_IO_INIT

@set KernelStackWalk=-stackwalk PROFILE+CSWITCH+READYTHREAD
@rem Disable stack walking if you want to reduce the data rate.
@if "%2" == "nostacks" set KernelStackWalk=
@set SessionName=UnityGameSession

@rem Set the buffer size to 1024 KB (default is 64-KB) and a minimum of 300 buffers.
@rem This helps to avoid losing events. Increase minbuffers if events are still lost,
@rem but be aware that the current setting locks up 300 MB of RAM.
@set KBuffers=-buffersize 1024 -minbuffers 300

@rem Stop the circular tracing if it is enabled.
@call etwcirc stop

@rem Start the kernel provider and user-mode provider
@echo CustomProviders are '%CustomProviders%'
@echo UserProviders are '%UserProviders%'
xperf -on %KernelProviders% %KernelStackWalk% %KBuffers% -start %SessionName% -on %UserProviders%%CustomProviders%
@if not %errorlevel% equ -2147023892 goto NotInvalidFlags
@echo Trying again without the custom providers. Run ETWRegister.bat to register them.
xperf -on %KernelProviders% %KernelStackWalk% %KBuffers% -start %SessionName% -on %UserProviders%
:NotInvalidFlags
@if not %errorlevel% equ 0 goto failure
@rem Record information about the loggers at the start of the trace.
xperf -loggers >preloggers.txt

@echo Run the test you want to profile here
@pause

@rem Record information about the loggers (dropped events, for instance)
xperf -loggers >loggers.txt
@rem Record the data and stop tracing
xperf -stop %SessionName% -stop -d %FileName%
@if not %errorlevel% equ 0 goto FailureToRecord
@rem Delete the temporary ETL files
@del \*.etl
@echo Trace data is in %FileName% -- load it with wpa or xperfview or gpuview. Logger details are in preloggers.txt and loggers.txt
start wpa %FileName%
@rem Restart circular tracing.
@call etwcirc StartSilent
@exit /b

:FailureToRecord
@rem Delete the temporary ETL files
@del \*.etl
@echo Failed to record trace.
@rem Restart circular tracing.
@call etwcirc StartSilent
@exit /b

:failure
@rem Check for Access Denied
@if %errorlevel% == %ACCESSISDENIED% goto NotAdmin
@echo Failed to start tracing. Make sure the custom providers are registered
@echo (using etwregister.bat) or remove the line that adds them to UserProviders.
@echo Make sure you are running from an elevated command prompt.
@echo Forcibly stopping the kernel and user session to correct possible
@echo "file already exists" errors.
xperf -stop %SessionName% -stop
@exit /b

:NotAdmin
@echo You must run this batch file as administrator.
@exit /b