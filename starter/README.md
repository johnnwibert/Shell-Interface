# Shell

A Unix-style shell written in C that supports prompt display, environment
variable and tilde expansion, `$PATH` search, external command execution,
I/O redirection, background processing, and built-in commands (`cd`,
`exit`, `jobs`).

## Group Members
- John Wibert: jnw22c@fsu.edu
- Armani Ruiz: arj22c@fsu.edu
- Robert Began: rgb23a@fsu.edu
  
## Division of Labor

### Part 0: Tokenization
Responsibilities: Provided base lexer (`get_input`, `get_tokens`, tokenlist struct) from the course starter files.
Assigned to: John Wibert

### Part 1: Prompt
- **Responsibilities**: Displays `USER@MACHINE:PWD>' prompt using environment variables; owns the main loop.
- **Assigned to**: John Wibert

### Part 2: Environment Variables
- **Responsibilities**: Expands `$VAR` tokens to their environment values.
- **Assigned to**: John Wibert

### Part 3: Tilde Expansion
- **Responsibilities**: Expands `~` and `~/...` tokens to `$HOME`.
- **Assigned to**: Armani Ruiz

### Part 4: $PATH Search
- **Responsibilities**: Searches `$PATH` directories to resolve a command name to an executable path.
- **Assigned to**: Armani Ruiz

### Part 5: External Command Execution
- **Responsibilities**: Forks a child process and uses `execv()` to run external commands
- **Assigned to**: Armani Ruiz

### Part 6: I/O Redirection
- **Responsibilities**: Parses `<` and `>` out of the token list, opens files with correct permissions (`-rw-------` on output), and redirects stdin/stdout before exec.
- **Assigned to**: Robert Began

### Part 7: Piping
- **Responsibilities**: Splits token list into multiple commands if one or two pipes (`|`) are identified. The standard stdout of one command is used as the stdin for the next.
- **Assigned to**: Armani Ruiz, John Wibert

### Part 8: Background Processing
- **Responsibilities**: Tracks background jobs with incrementing job numbers, prints start/done messages, and reaps finished jobs each time the prompt is shown.
- **Assigned to**: Robert Began

### Part 9: Internal Command Execution
- **Responsibilities**: Implements `cd`, `exit`, and `jobs` as built-ins that run inside the shell process rather than through `execv()`.
- **Assigned to**: Robert Began

## File Listing
```
shell/
│
├── src/
│ ├── lexer.c — main loop, prompt, tokenizer, env/tilde expansion
│ ├── path.c — $PATH search (Part 4)
│ ├── piping.c — piping implemented (Part 7)
│ ├── e_execute.c — external command execution (Part 5), calls into redirect/jobs
│ ├── redirect.c — I/O redirection (Part 6)
│ ├── Jobs.c — background job tracking (Part 8)
│ ├── Builtins.c — cd, exit, jobs built-ins (Part 9)
│ ├── History.c — tracks last 3 valid commands for exit
│ └── Util.c — shared allocation helpers
│
├── include/
│ ├── lexer.h
│ ├── path.h
│ ├── piping.h
│ ├── e_execute.h
│ ├── redirect.h
│ ├── jobs.h
│ ├── builtins.h
│ ├── history.h
│ └── util.h
│
├── README.md
└── Makefile
```
## How to Compile & Execute

### Requirements
- Compiler: `gcc`
- No external libraries required.

### Compilation
For a C/C++ example:
```bash
make
```
This builds the executable at `bin/shell`.
### Execution
```bash
./bin/shell
```
or the below
```bash
make run
```
## Development Log
### Robert Began
| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-09-25 | Got background jobs working (Part 8) - job numbers print when a command starts with `&`, and it prints "done" once they finish. Also added `cd`, `exit`, and `jobs` as built-in commands (Part 9). Merged in Armani's branch for $PATH search and running external commands. |
| 2026-09-25 | Added I/O redirection (Part 6) - `<` and `>` now work and create files with the right permissions. Connected everything (redirection, background jobs, built-ins) into the main loop so it actually runs. |
| 2026-09-27 | Tested everything together (redirection, background jobs, built-ins) to make sure it all still works. Fixed a bug where a file didn't save right, and removed some leftover debug print statements before merging. |

### Armani Ruiz

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-09-20 | Added support for tilde expansion in `lexer.c`; `~` now expands into the environment variable $HOME. Later fixed some minor syntax errors with this implementation.  |
| 2026-09-21 | Implemented $PATH search to find and identify commands, and temporarily modified `main()` in `lexer.c` to test this implementation.  |
| 2026-09-23 | Implemented the first iteration of external command execution to execute commands located by $PATH search, and temporarily modified `main()` in `lexer.c` to test this implementation.  |
| 2026-09-25 | Fixed a small bug with my external command execution that was causing the shell to crash under certain circumstances.  |
| 2026-09-27 | Addressed some merge conflicts when attempting to merge Robert's changes to main. |
| 2026-09-28 | Began working on piping before handing the rest off to John. Token list is now split into multiple commands if one or two pipes are present.  |

### John Noel Wibert

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-09-18 | Set up the starter files and uploaded them to Github. Completed tasks 1 and 2. |
| 2026-09-28 | Continued off Armani's progress on Part 7 to complete it and fixed miscellaneous minor bugs.  |


## Considerations
- Redirection requires spaces around `<` and `>` (e.g. `echo hi > out.txt`, not `echo hi >out.txt`), since the tokenizer only splits on whitespace.
- Background job numbers increment and are never reused, per spec, but are capped at 10 total per shell session per the assignment's stated assumption of at most 10 concurrent background jobs.
