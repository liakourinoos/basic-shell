
# MySH – Simple Unix Shell (System Programming Assignment)

This project is an implementation of a basic Unix shell for the *System Programming* course. All required features are specified in the `hw1-spring-2023.pdf` assignment description.

### 🛠 Build & Run
- **Build:** `make`  
- **Run:** `./mysh`  
- **Exit shell:** `quit`

---

## 🧠 Overview

The shell reads a user input string containing one or more commands (separated by `;`) and tokenizes it into a 3D `char ***command` array, where `command[i][j]` corresponds to the `j`-th word of the `i`-th command.

Memory usage is a bit generous: each command is allocated as a `MAXSIZE × MAXSIZE` grid. However, memory is freed properly, and there are **no memory leaks**.

- Pipelined commands are split into their components and executed separately.
- Aliases and PID tracking use **linked lists**.
- Each alias is a `struct` containing its name and a `MAXSIZE × MAXSIZE` command array.

---

## 💡 Design Notes

- **Input assumptions:**
  - User input is syntactically valid.
  - Only **double quotes** (`"`) are supported (for `createalias` only).
  - Environment variables are not supported except for `cd`, which defaults to `$HOME` if no argument is provided.

- **Pipelining support:** Works for built-in commands like `myHistory` and `aliases`.
- **Command history:** Multiple commands entered in a single line are stored individually.
- **Zombie handling:** Background processes are tracked and cleaned using a global PID list.

---

## 📚 Component Breakdown

### `main`  
Calls `shell()`, which starts the shell loop.

### `shell()`  
- Initializes alias and zombie PID lists.
- Initializes a `20 × MAXSIZE × MAXSIZE` history array.
- Continuously:
  - Reads user input.
  - Parses it into the `command` array.
  - Executes each command.
  - Cleans up based on whether the process is parent/child.
  - Uses `indexes_array` to track word counts per command.

### `execute()`  
- Detects command type and executes accordingly.
- Handles alias replacement, using pointers to `command`, `indexes_array`, and command count.
- For piped commands:
  - Splits and stores components in a temporary array.
  - Executes them without blocking until all processes are launched.
- For background execution (`&`):
  - Uses process groups for better signal management.
  - Avoids `wait()` for background jobs; cleaned up in the next shell iteration.

### `seperate_commands()`  
Breaks down a single pipelined command into components stored in a temporary array.

### `open_redirections()`  
Handles input/output redirection by opening the relevant files and removing redirection tokens from the final `execvp` call.

### `history_*()`  
- `history_get()`: Retrieves and substitutes a past command by index.
- `history_update()`: Appends a new command to history if it's not a duplicate.
- `history_init()` / `history_destroy()`: Allocates and frees history memory.

### `input_parse()`  
- Splits input by `;` and spaces.
- Handles quoted segments by temporarily substituting spaces with `/` during parsing.
- Uses stored positions to restore original spaces after parsing.

### `check_correct_input()`  
Basic validation to reject malformed commands before parsing.

### `alias_management()`  
Handles creation, deletion, or updating of aliases.

### `get_alias()`  
- Locates and replaces an alias in `command[]`.
- If the alias includes multiple commands, creates a new expanded `command` array.

### `list_append()`  
Adds a new alias to the linked list.

### `check_for_wildchars()`  
- Detects wildcards and expands them using `glob()`.
- Assumes final word count post-expansion is below `MAXSIZE`.

### `check_for_zombies()`  
- Frees zombie processes using a global PID list.
- Called before each command to prevent resource leaks.

---

## 🔍 Utility Functions
- `search_in_array()`: Searches for a word in a command.
- Linked list utilities: insert, delete, search, count, init, destroy.
