# Vortex

Vortex is a language designed to explore links and relationships between entities.

It heavily relies on side effects, and as such it would fall under the Dysfunctional Programming paradigm.

## About Vortex

Vortex's main focus is on what we call "hooks". Hooks allow the user to bind variables to functions in the context of specific events. 

For example, if we wanted automatically increment variable y whenever variable x changes, we could use the onChange hook to do so:

```go
var x = 0
var y = 0

x::onChange = () => {
    y += 1
}

x = 2 // y == 1
x = 4 // y == 2
```

We could also implement hooks for object properties:

```go
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

```go
var x = 0

x::onChange = (new, old, info) => println(info.name + ": " + string(old) + " -> " + string(new))

x = 20 // x: 0 -> 20
```

We'll cover what data can be extracted and used within each hook in a later section.

### Multi-variable Hooks

For added convenience, we can also provide a list of variables for which to implement a specific hook for:

```go
var xPos = 0
var yPos = 0

[xPos, yPos]::onChange(old, new, info) => println(info.name + " has changed")

xPos = 2 // x has changed
yPos = 3 // y has changed
```

### Global Hooks

We can also set up hooks globally, meaning that the hook will apply to all variables who are bound by the hook's context (onChange, onCalled etc.). We do this by using empty brackets:

```go
var x = 0
var y = 0
var z = 0

[]::onChange = (old, new, info) => println(info.name + " has changed")

x = 25  // x has changed
y = -54 // y has changed
```

### Named Hooks

Up until now, we've only seen hooks being implemented once in a set and forget approach. But hooks can be more powerful than that. Named hooks allow the user to create hooks that can be switched on and off, either globally or for specific variables:

```go
const logOnChange = onChange::(new, old, info) => 
    println(info.name + ": " + string(old) + " -> " + string(new))

var x = 0
var y = 0
var z = 0

logOnChange.attach([x, z]) // This attaches the onChange hook to x and z

x = 10
y = 18
z = 20

// x: 0 -> 10
// z: 0 -> 20

logOnChange.detach(z) // This detaches the onChange hook from z

x = 100
y = 180
z = 200

// x: 20 -> 200

logOnChange.attach() // This attaches the onChange hook globally

x = 1000
y = 1800
z = 2000

// x: 100 -> 1000
// x: 100 -> 1000
// y: 180 -> 1800
// z: 200 -> 2000

// Important note: Notice how the onChange hook fired twice for x. This is because the current implementation of named hooks simply adds that hook to the variable, and because the same hook existed on x, it fired twice. This behaviour may change in future releases. For the time being, it's best to call a global detach before a global attach.

```

This allows for a more programatic and strategic approach to using hooks.

### Hook types

**onChange**: This hook will fire whenever a variable changes.\
**onChange parameters**: *new*: The new value, *old*: The old value, *info*: An object containing further information about the variable and context, the properties are (*name*: Name of the variable, *filename*: Name of the file, *line*: File line number, *column*: File column number)

**onCall**: This hook will fire whenever a function is called.\
**onCall parameters**: *info*: An object containing further information about the variable and context, the properties are (*name*: Name of the function, *args*: A list containing the arguments the function was called with, *result*: The result of the function call, *filename*: Name of the file, *line*: File line number, *column*: File column number)

### Imports

In order to allow for best practices and modularity, Vortex allows two types of imports: module and variable

## Module Import

Module imports allow the user to import an entire file (module) into the current scope. The imported module can be used as an object:

```go
import math : "../modules/math"

const res = math.fib(10)
```

## Variable Import

Variable imports allow the user to pick and choose what they want to import into the local scope:

```go
import [PI, e, fib] : "../modules/math"

const res = PI * e + fib(20)
```

Vortex's imports use relative paths to retrieve modules.

## How to start using Vortex

You can find the [full Vortex documentation here](https://dibs.gitbook.io/vortex-docs/). This includes steps on how to get started using Vortex on your local machine.

## Basic example

```go
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

```go
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
