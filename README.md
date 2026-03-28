# 🐚 MiniShell – Custom Linux Shell in C

A lightweight **MiniShell** built in C that mimics basic functionalities of a Linux shell.
It supports **internal commands, external commands, piping, signal handling, and job control**.

---

## 🚀 Features

✨ Interactive command-line shell
⚙️ Supports **built-in commands** (cd, pwd, echo, exit, etc.)
📂 Executes **external Linux commands**
🔗 Handles **multiple pipes (`|`)**
🛑 Supports **job control (fg, bg, jobs)**
🎯 Signal handling for:

* `Ctrl + C` (SIGINT)
* `Ctrl + Z` (SIGTSTP)
* Background process tracking (SIGCHLD)
  🎨 Customizable shell prompt (`PS1`)

---

## 🛠️ Built-in Commands

| Command | Description                   |
| ------- | ----------------------------- |
| `cd`    | Change directory              |
| `pwd`   | Print current directory       |
| `echo`  | Display environment variables |
| `exit`  | Exit the shell                |
| `jobs`  | List stopped jobs             |
| `fg`    | Bring job to foreground       |
| `bg`    | Resume job in background      |

---

## 💻 External Commands

The shell supports external commands listed in `external.txt` such as:

```
ls, cat, pwd, grep, mkdir, rm, cp, mv, nano, ping, etc.
```

These are executed using `execvp()`.

---

## 🔗 Pipe Support

You can run multiple commands using pipes:

```
ls | grep .c | wc
```

✔ Supports multiple chained pipes
✔ Handles input/output redirection internally

---

## 🎯 Signal Handling

The shell properly handles:

* **SIGINT (`Ctrl + C`)**

  * Stops running process without exiting shell
* **SIGTSTP (`Ctrl + Z`)**

  * Moves process to background (stopped state)
* **SIGCHLD**

  * Tracks child process termination

---

## 🧠 Job Control

* Stores stopped processes
* Resume using:

  ```
  fg
  bg
  ```
* View jobs:

  ```
  jobs
  ```

---

## 🎨 Custom Prompt

You can change the shell prompt using:

```
PS1=myshell
```

Example:

```
myshell: ls
```

---

## 🧪 How to Compile & Run

### 🔧 Compile

```bash
gcc *.c -o minishell
```

### ▶️ Run

```bash
./minishell
```

---

## 📂 Project Structure

```
├── main.c
├── main.h
├── external.txt
├── other source files
```

## 📚 Concepts Used

* 🧵 Process Management (`fork`, `exec`, `wait`)
* 🔁 Pipes & IPC
* ⚡ Signal Handling
* 🧠 String Parsing
* 📂 File Handling
* 🖥️ System Calls

---

## 👨‍💻 Author

N.Mohammed Shaqeeb

---
