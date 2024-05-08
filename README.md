# Wish-shell

Developed a simple similar Unix shell named Wish. The shell is the heart of
the command-line interface, and thus is central to the Unix/C programming
environment.


## Overview

Implement a *command line interpreter (CLI)* or,
as it is more commonly known, a *shell*. This shell operates in this
basic way: when you type in a command (in response to its prompt), the shell
creates a child process that executes the command you entered and then prompts
for more user input when it has finished.

The shell implemented is similar and simpler than, the one we use in Unix.

## Program Specifications

### Basic Shell: `wish`

Basic shell, called `wish` is an 
interactive loop: it repeatedly prints a prompt `wish> `, parses the input, executes the command
specified on that line of input, and waits for the command to finish. This is
repeated until the user types `exit`.

The shell can be invoked with either no arguments or a single argument;
anything else is an error. Here is example of the no-argument way:

```
prompt> ./wish
wish> 
```

The mode above is called *interactive* mode, and allows the user to type
commands directly. The shell also supports a *batch mode*, which instead reads
input from a batch file and executes commands from therein. Here is how you
run the shell with a batch file named `batch.txt`:

```
prompt> ./wish batch.txt
```

One difference between batch and interactive modes: in interactive mode, a
prompt is printed (`wish> `). In batch mode, no prompt is printed.

## Structure

### Wish Shell

Shell runs in a while loop, repeatedly
asking for input to tell it what command to execute. It then executes that
command. The loop continues indefinitely, until the user types the built-in
command `exit`, at this point it exits.

Shell creates a process for each new command (except *built-in commands*). 

### Paths

A **path** variable is specified to describe the set of directories to 
search for executables; the set of directories that comprise the path are
called the *search path* of the shell. The path variable contains the list 
of all directories to search, in order, when the user types a command. 

**Important:** Note that the shell itself does not *implement* `ls` or other
commands (except built-ins). All it does is find those executables in one of
the directories specified by `path` and create a new process to run them.

Initial shell path contains one directory: `/bin`

### Built-in Commands

Whenever shell accepts a command, it checks whether the command is
a **built-in command** or not. If it is, it is not be executed like other
programs. Instead, shell will invoke implementation of the built-in
command.

Wish Shell supports `exit`, `cd`, and `path` as built-in
commands.

* `exit`: When the user types `exit`, shell simply call the `exit`
  system call with 0 as a parameter. It is an error to pass any arguments to
  `exit`. 

* `cd`: `cd` always take one argument (0 or >1 args is signaled as an
error). To change directories, use the `chdir()` system call with the argument
supplied by the user; if `chdir` fails, that is also an error.

* `path`: The `path` command takes 0 or more arguments, with each argument
  separated by whitespace from the others. A typical usage would be like this:
  `wish> path /bin /usr/bin`, which would add `/bin` and `/usr/bin` to the
  search path of the shell. If the user sets path to be empty, then the shell
  is not be able to run any programs (except built-in commands). The
  `path` command always overwrites the old path with the newly specified
  path. 

### Redirection

Many times, a shell user prefers to send the output of a program to a file
rather than to the screen. Redirection using the`>` character is implemented 
with a slight change.

For example, if a user types `ls -la /tmp > output`, nothing is printed
on the screen. Instead, the standard output of the `ls` program is being
rerouted to the file `output`. In addition, the standard error output of
the program is rerouted to the file `output` (this
is a little different than standard redirection).

### Parallel Commands

Shell also allows the user to launch parallel commands. This is
accomplished with the ampersand operator as follows:

```
wish> cmd1 & cmd2 args1 args2 & cmd3 args1
```

In this case, instead of running `cmd1` and then waiting for it to finish,
shell run `cmd1`, `cmd2`, and `cmd3` (each with whatever arguments
the user has passed to it) in parallel, *before* waiting for any of them to
complete. 

Then, after starting all such processes, shell make sure to use `wait()`
(or `waitpid`) to wait for them to complete. After all processes are done,
return control to the user as usual (or, if in batch mode, move on to the next
line).


### Program Errors

Whenever shell encounters an error of any type, one and only error message is printed:

```
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
```

The error message is printed to stderr (standard error), as shown
above. 

After most errors, shell simply *continue processing* after
printing the one and only error message. However, if the shell is
invoked with more than one file, or if the shell is passed a bad batch
file, it terminates by calling `exit(1)`.

### Miscellaneous features

Shell is robust to white space of various kinds, including spaces (` `) and 
tabs (`\t`). The user can put variable amounts of white 
space before and after commands, arguments, and various operators.
