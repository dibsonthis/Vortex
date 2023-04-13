# Ripple

Ripple (name uncomfirmed) is a language designed to explore links and relationships between entities.

It heavily relies on side effects, and as such it would fall under the Disfunctional Programming paradigm.

## Basic example

```
var length, area, diff = 0

length::onChange = () => {
	area = length ^ 2
}

area::onChange = (old) => {
	diff = area - old
}

for (1..10) {
	length = length + 1
	print("L: " + string(length) + 
		  " - A: " + string(area) + 
		  " - D: " + string(diff) + "\n")
}
```

## Output

```
L: 1.000000 - A: 1.000000 - D: 1.000000
L: 2.000000 - A: 4.000000 - D: 3.000000
L: 3.000000 - A: 9.000000 - D: 5.000000
L: 4.000000 - A: 16.000000 - D: 7.000000
L: 5.000000 - A: 25.000000 - D: 9.000000
L: 6.000000 - A: 36.000000 - D: 11.000000
L: 7.000000 - A: 49.000000 - D: 13.000000
L: 8.000000 - A: 64.000000 - D: 15.000000
L: 9.000000 - A: 81.000000 - D: 17.000000
```

More to come soon...
