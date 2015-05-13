def f(x: Int, g: Int => Int = _ + 1) = g(x)

// Examples:
//
// f(3)
// res8: Int = 4
//
// scala> f(3, _ + 2)
// res9: Int = 5


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def applyFunc1(x: Int, f: Int => Int) = f(x)

// option 1 (placeholder syntax directly in call):
applyFunc1(5, _ + 1)

// option 2 (placeholder syntax in function defined separately):
def func1a: Int => Int = _ + 1
applyFunc1(5, func1a)

// option 3 (without placeholder syntax):
def func1b: Int => Int = (x: Int) => x + 1
applyFunc1(5, func1b)


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def applyFunc2(x: Int, y: Int, f: (Int, Int) => Int) = f(x, y)

// option 1 (placeholder syntax directly in call):
applyFunc2(5, 7, _ + _)

// option 2 (placeholder syntax in function defined separately):
def func2a: (Int, Int) => Int = _ + _
applyFunc2(5, 7, func2a)

// option 3 (without placeholder syntax):
def func2b: (Int, Int) => Int = (x: Int, y: Int) => x + y
applyFunc2(5, 7, func2b)
