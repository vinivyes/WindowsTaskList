# Windows Task List
Get a list of Processes with the respective Command Line in Windows without relying on WMI.

## Why ?
Currently, if you are trying to list processes to retrieve Command Line on a machine where WMI is not running, you must resort to different tools that might not easily integrate with other applications. The idea is to have a tool that can return Command Line information regardless of state of WMI.

## How to use:
- TaskList.exe
> Will retrieve a list of all processes returning, PID and Command Line
- TaskList.exe "myProgram.exe" 
> Will retrieve any processes from executable myProgram.exe returning PID and Command Line.
## DISCLAIMER:
This has not been widely tested.
