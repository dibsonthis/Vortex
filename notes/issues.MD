# Known Issues:

- creating a closure from a variable inside an if statement causes undefined behaviour, due to that variable going out of scope -- luckily this isn't really heavily used, but it's still a major bug that needs to be fixed
- there is no try/catch or exception handling
- errors aren't fantastic -- need to create an Error object that can get returned