# Ripple

Ripple is a language designed to explore links and relationships between entities.

It heavily relies on side effects, and as such it would fall under the Disfunctional Programming paradigm.

## About Ripple

Ripple's main focus is on what we call "hooks". Hooks allow the user to bind variables to functions in the context of specific events. 

For example, if we wanted automatically increment variable y whenever variable x changes, we could use the onChange hook to do so:

```
var x, y = 0

x::onChange = () => {
    y += 1
}

x = 2 // y == 1
x = 4 // y == 2
```

We could also implement hooks for object properties:

```
var person = {
    name: "John"
}

person.name::onChange = () => {
    println("Person's name has changed!")
}

person.name = "Allan" // Person's name has changed!
```

### Hook parameters

Hooks also expose data about the current event. For example, we can extract and use a variable's old and new values inside the onChange hook:

```
var x = 0

x::onChange = (new, old, info) => println(info.name + ": " + string(old) + " -> " + string(new))

x = 20 // x: 0 -> 20
```

We'll cover what data can be extracted and used within each hook in a later section.

### Multi-variable Hooks

For added convenience, we can also provide a list of variables for which to implement a specific hook for:

```
var xPos, yPos = 0

[xPos, yPos]::onChange(old, new, info) => println(info.name + " has changed")

x = 2 // x has changed
y = 3 // y has changed
```

### Global Hooks

We can also set up hooks globally, meaning that the hook will apply to all variables who are bound by the hook's context (onChange, onCalled etc.). We do this by using empty brackets:

```
var x, y, z = 0

[]::onChange = (old, new, info) => println(info.name + " has changed")

x = 25  // x has changed
y = -54 // y has changed
```

### Hook types

**onChange**: This hook will fire whenever a variable changes.
**onChange parameters**: *new*: The new value, *old*: The old value, *info*: An object containing further information about the variable and context, the properties are (*name*: Name of the variable, *filename*: Name of the file, *line*: File line number, *column*: File column number)

**onCall**: This hook will fire whenever a function is called.
**onCall parameters**: *info*: An object containing further information about the variable and context, the properties are (*name*: Name of the variable, *args*: A list containing the arguments the function was called with, *result*: The result of the function call)
## Development Roadmap

- [x] Basic operations
- [x] Lists
- [x] Objects
- [x] Functions and function calls
- [x] Function currying
- [x] Branching
- [x] For loops
- [x] While loops
- [x] List functions
- [x] Imports
- [x] Builtins
- [x] Hook: onChange
- [x] Hook: onCall
- [ ] Hook: onDelete
- [ ] Hook: onUse
- [ ] I/O
- [ ] Named hooks
- [ ] String interpolation
- [ ] C/C++ Interoperability

## Basic example

```
var length, area = 0

length::onChange = () => {
	area = length ^ 2
}

for (1..10) {
    length += 1
    print("L: " + string(length) + "\tA: " + string(area))
}
```

## Output

```
L: 1    A: 1
L: 2    A: 4
L: 3    A: 9
L: 4    A: 16
L: 5    A: 25
L: 6    A: 36
L: 7    A: 49
L: 8    A: 64
L: 9    A: 81
```

## Slightly more complex example

```
var lexer = {
    index: -1,
    tokens: ["const", "x", "=", 9, 10, "5", [1, 2, 3]],
    currentNode: this.tokens[0]
}

lexer.index::onChange = (curr) => {
    lexer.currentNode = lexer.tokens[curr]
}

lexer.currentNode::onChange = (curr) => {
    if (curr != string(curr)) {
    	println("[Removing non-string token: " + string(curr) + "]")
    	lexer.tokens.remove_at(lexer.index)
    	lexer.index -= 1
}
}

println(lexer)

for (0..(lexer.tokens.length)) {
    lexer.index += 1
}

println(lexer)
```

## Output

```
{ currentNode: const tokens: [const, x, =, 9, 10, 5, [1, 2, 3]] index: -1 }
[Removing non-string token: 9]
[Removing non-string token: 10]
[Removing non-string token: [1, 2, 3]]
{ currentNode: 5 tokens: [const, x, =, 5] index: 3 }
```
