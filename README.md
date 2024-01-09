# Vortex

Vortex is a language designed to explore links and relationships between entities.

It heavily relies on side effects, and as such it would fall under the Dysfunctional Programming paradigm.

## About Vortex

Vortex is a dynamically typed programming language that comes with C interoperability out of the box.

## Start Using Vortex Now

If you want to start using Vortex straight away, we've pre-compiled the intepreter and standard library modules for an easy to use seamless experience. Running the below will create a self contained scaffold in the current directory that includes everything you need to run Vortex programs, namely:

- The Vortex interpreter
- Vortex standard library modules
- Vortex config file
- Basic source directory scaffold
- A run script to launch your application

If you wish to make changes to the scaffolded directory, make sure to update the config file to point to the correct entry point.

To run the program, simply call `./run` in the root directory. This will launch the intepreter with the provided entry point.

To create an isolated self contained Vortex environment, simply cd into your desired directory and run the following code:

Mac:
```bash
curl https://raw.githubusercontent.com/dibsonthis/vortex-dist/main/mac/scaffold | bash
```

Linux:
```bash
curl https://raw.githubusercontent.com/dibsonthis/vortex-dist/main/lin/scaffold | bash
```

Windows:
```bash
curl -LO -k https://raw.githubusercontent.com/dibsonthis/vortex-dist/main/win/scaffold.bat
scaffold.bat
```

More information cam be found here: [Vortex Dist](https://github.com/dibsonthis/vortex-dist)

## Features

Alongside the standard functionality that most programming languages provide (eg. branching, loops, lists, objects etc.), Vortex also provides:

- C interop
- First class functions
- Decorators
- Recursion
- Coroutines
- Hooks

## Hooks

Vortex allows you to bind variables to functions that get called whenever the variable changes. These functions are called hooks. Currently, Vortex has support for only one type of hook: 'onChange'.

(Note: Previous versions of Vortex supported an 'onCall' hook that could be bound to functions, however this has been deprecated in favour of using decorators for this purpose)

For example, if we wanted automatically increment variable y whenever variable x changes, we could use the 'onChange' hook to do so:

```go
var x = 0
var y = 0

x::onChange((info) => {
    y += 1
})

x = 2 // y: 1
x = 4 // y: 2
```

Hooks can also be applied to object properties. In the example below, we're defining a type (really, it's just a function that returns an object, but with slight differences) named 'Color'. Inside this type, we create a function that caps an input to a low and high number. We apply this function to all three arguments passed in to the constructor (r, g, b) to make sure they are capped on init. After that, we assign an 'onChange' hook to each argument, making sure we run the cap function to it each time it changes. Finally, we build the object and return it.

This ensures that any object instantiated with the constructor 'Color' has constraints that the hook deals with internally. As you can see, adding 400 to 'r' caps it to 255, and subtracting 300 from 'g' caps it to 0, without the user having to do any checks on their behalf.

```go
type Color = (r, g, b) => {
    const cap = (num, low, high) => {
        if (num > high) {
            return high
        } else if (num < low) {
            return low
        }
        return num
    }

    r = cap(r, 0, 255)
    g = cap(g, 0, 255)
    b = cap(b, 0, 255)

    r::onChange((info) => {
        info.current = cap(info.current, 0, 255)
    })

    g::onChange((info) => {
        info.current = cap(info.current, 0, 255)
    })

    b::onChange((info) => {
        info.current = cap(info.current, 0, 255)
    })

    return { r: r, g: g, b: b }
}

var color = Color(100, 200, 234)
color.r += 400
color.g -= 300

print(color) // { r: 255.000000, g: 0.000000, b: 234.000000 }
```

The function provided to the onChange hook (the function that gets called when the bound variable changes) must have one parameter. This parameter becomes an object at runtime containing the old value and the current value. In the example above, the 'info' parameter is an object that contains the properties 'old' and 'current' that can be references within the hook function.

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

## How to compile and use Vortex

Clone the repo.

Go to the root of the directory.

<i>[Mac/Linux]</i> Run the build script: build.sh (requires clang) \
<i>[Windows]</i> Run the build script: build.bat (requires g++) (Support for Windows version is currently extremely limited and mostly untested)

If you're on Mac/Linux, the installer will ask if you want to store the interpreter in usr/local/bin (and set up modules in usr/local/share). This allows you to call Vortex from anywhere. If you choose yes, you'll need to input your password to continue.

<b>Note:</b> If you choose to add Vortex in your bin folder (and modules in share), you can later remove them by running the uninstall.sh script found in usr/local/share/vortex

Depending on your system, the interpreter would have compiled in one of the system folders in bin/build (either mac, windows or linux). The executable we just compiled (vortex) is the interpreter that we'll be using to run our Vortex code.

If you chose not to add Vortex to your bin folder, you can manually add it to your PATH so you can call it globally. Otherwise, you'll need to either store it somewhere accessible or within your project.

Choosing not to add Vortex to your PATH when building means the standard library modules will also not be added and therefore cannot be used.

## Your first Vortex program

Let's write a very quick Vortex program that defines some functions and calls them in a loop.

Start by creating a vortex source file (a file ending in the extension .vtx) and paste the below code in:

```go
const addVars = (a, b) => a + b
const mulTwo = (a) => a * 2

for (0..5, index) {
    const add = addVars(index, index + 1)
    const res = mulTwo(add)
    print(res, "\n")
}

print("Obligatory 'Hello World'\n")
```

Run the above Vortex program by calling: vortex <path/to/dir>/hello.vtx

This is the expected output:

```go
2
6
10
14
18
Obligatory 'Hello World'
```

Congratulations! You just ran your first Vortex code 🎉

<!-- ## How to start using Vortex

You can find the [full Vortex documentation here](https://dibs.gitbook.io/vortex-docs/). This includes steps on how to get started using Vortex on your local machine. -->

## Examples

### Priority Queue

```go

// Define a priority queue constructor

const PriorityQueue = (max = None) => ({
    data: [],
    max: max,
    enqueue: (value, priority = 0) => {
        if (this.max == None) {
            var _value = [value, priority]
            this.data.append(_value)
            this.data = this.data.sort((a, b) => a[1] < b[1])
        } else {
            if (this.data.length() < this.max) {
                var _value = [value, priority]
                this.data.append(_value)
                this.data = this.data.sort((a, b) => a[1] < b[1])
            }
        }
    },
    dequeue: () => {
        var value = this.data[0][0]
        this.data.remove(0)
        if (value == None) {
            return None
        }
        return value
    },
    clear: () => {
        this.data = []
    },
    size: () => this.data.length(),
    front: () => (this.data[0])[0],
    back: () => (this.data[this.data.length()-1])[0]
})

const pq = PriorityQueue()

const Priority = {
    CRITICAL: 0,
    ERROR: 1,
    DEBUG: 2,
    INFO: 3
}

pq.enqueue("Logging in", Priority.INFO)
pq.enqueue("Encoutered a critical error", Priority.CRITICAL)
pq.enqueue("Uh ohhhh", Priority.ERROR)
pq.enqueue("Logging out", Priority.INFO)
pq.enqueue("Encountered bug", Priority.DEBUG)
pq.enqueue("The end is here", Priority.CRITICAL)

while (pq.size() > 0) {
    println(pq.dequeue())
}

/*
Output:

Encoutered a critical error
The end is here
Uh ohhhh
Encountered bug
Logging in
Logging out
*/
```

### Decorators

```go
const Timer = (func) => {
    return (...args) => {
        const start = clock()
        var value = func(...args)
        const duration = clock() - start
        print(f"Duration (${func.info().name}): ", duration, "\n")
        return value
    }
}

@Timer
const somefunc = () => {
    var i = 0
    for (0..100) {
        for (0..100) {
            i += 1
        }
    }
    print(i, "\n")
}

somefunc()

/*
Output:

10000.000000
Duration (somefunc): 0.002987
*/
```