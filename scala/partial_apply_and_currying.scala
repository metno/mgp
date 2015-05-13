// *** partial function application ***

// basic function to add two numbers:
def add(x: Int, y: Int) = x + y

// option 1:
def add3(x: Int) = add(3, x)
// add3(4) yields 7

// option 2:
val add3 = add(3, _: Int)
// add3(4) yields 7


// *** currying ***

// basic function to add two numbers (this time with parameters organized into two groups):
def add(x: Int)(y: Int) = x + y

// option 1:
def add3(x: Int) = add(3)(x)
// add3(4) yields 7

// option 2:
val add3 = add(3) _
// add3(4) yields 7
