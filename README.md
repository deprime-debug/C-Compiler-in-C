# C-Compiler-in-C

---

```markdown
# C to x86-64 Mini Compiler

A lightweight, dependency-free C compiler written from scratch in C. It compiles a minimal subset of C code into system-native System V x86-64 assembly language.

## 🚀 Overview

This project implements a classic multi-stage compiler pipeline without external parsing libraries:
- **Lexical Analysis (Lexer):** Tokenizes input C source code while ignoring whitespace.
- **Abstract Syntax Tree (AST):** Builds a recursive tree structure representing expressions, function declarations, and return statements.
- **Code Generation (Codegen):** Emits AT&T-formatted x86-64 assembly, utilizing the execution stack (`pushq`/`popq`) to handle nested mathematical evaluations safely according to System V ABI calling conventions.

---

## 🛠️ Supported Features

- Function definitions (`int main() { ... }`)
- Return statements (`return <expr>;`)
- Integer literals
- Binary arithmetic operations (`+`, `-`, `*`)
- Parenthesized arithmetic sub-expressions

---

## 💻 Getting Started

### Prerequisites
- GCC or Clang
- Linux or macOS (x86-64 architecture, or Apple Silicon via Rosetta 2)

### Build and Run

1. **Compile the compiler:**
   ```bash
   gcc -Wall -Wextra main.c -o my_compiler

```

2. **Generate x86-64 assembly:**
```bash
./my_compiler > output.s

```


3. **Assemble into an executable binary:**
```bash
gcc output.s -o program

```


4. **Execute and check return value:**
```bash
./program
echo $?  # Displays the process exit code

```



---

## 📋 Example

### Input C Code

```c
int main() {
    return 42 + 8 * 2;
}

```

### Generated x86-64 Assembly (`output.s`)

```assembly
  .globl main
main:
  pushq %rbp
  movq %rsp, %rbp
  movq $42, %rax
  pushq %rax
  movq $8, %rax
  pushq %rax
  movq $2, %rax
  movq %rax, %rcx
  popq %rax
  imulq %rcx, %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
  movq %rbp, %rsp
  popq %rbp
  ret

```

---

## 📂 Project Structure

```text
.
├── main.c        # Lexer, Parser, AST, and Code Generator implementation
├── README.md     # Project documentation
└── .gitignore    # Ignores generated binaries and assembly outputs

```

## 📜 License

MIT

```

```
