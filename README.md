## Description
This *shell project* is the final project of 'Sistemas Operativos' class.

## Requisites
The follow requisites must be executed by the **shell** program:

### Command-line
- Execution of a command with an indeterminate number of arguments: *command arg1 arg2 ...*
- Execution of a command line made up of a pipeline with indeterminate length: *cmd1 arg11 ... | cmd2 arg21 arg22 ... | cmd3 arg31 ... | ...*
- Execution of a command with redirections: *cmd1 arg1 ... < file* or *cmd1 arg1 ... > file*
- Possibility of executing a command in background with ampersand symbol (&) at the end of the command line
- Possibility of definning environment variables with equal symbol (=), and refering to them with dollar symbol ($)

### Built-ins
Also, the program has some built-in commands:
- **cd** : change working directory
- **exit** : end *shell* program
