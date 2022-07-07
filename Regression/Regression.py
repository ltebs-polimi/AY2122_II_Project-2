import matplotlib.pyplot as plt
from scipy import stats

x = [80, 80, 80, 115, 115, 115, 150, 150, 150]
y = [1.9,
     2.05,
     1.65,
     2.3,
     2.25,
     2.3,
     2.65,
     2.5,
     2.6]

x2 = [80, 100, 115, 115, 150]
y2 = [2.1,
      2.55,
      2.70,
      3,
      4.3]

x3 = [80, 80, 115, 115, 150, 150]
y3 = [1.6, 1.75, 1.95, 1.9, 2.2, 2.45]

slope, intercept, r, p, std_err = stats.linregress(x, y)
slope2, intercept2, r2, p2, std_err2 = stats.linregress(x2, y2)
slope3, intercept3, r3, p3, std_err3 = stats.linregress(x3, y3)


def myfunc(x):
    return slope * x + intercept


def myfunc2(x2):
    return slope2 * x2 + intercept2


def myfunc3(x3):
    return slope3 * x3 + intercept3


mymodel = list(map(myfunc, x))
print("\nSlope: ")
print(slope)
print('Intercept: ')
print(intercept)

gridx = []
for i in range(80, 155, 5):
    gridx.append(i)

plt.figure(0)
plt.scatter(x, y, color='royalblue')
plt.plot(x, mymodel, color='red')
plt.ylabel('CA result [uA]')  # labeling the y label
plt.xlabel('Glucose concentration [mg/dL]')
plt.grid()
plt.xticks(gridx)
plt.title("Linear regression on collected data")
plt.legend(["Data points", "Regressor"], loc="upper left")

mymodel2 = list(map(myfunc2, x2))
print("\nSlope2: ")
print(slope2)
print('Intercept2: ')
print(intercept2)

gridx = []
for i in range(80, 155, 5):
    gridx.append(i)

plt.figure(1)
plt.scatter(x2, y2, color='royalblue')
plt.plot(x2, mymodel2, color='red')
plt.ylabel('CA result [uA]')  # labeling the y label
plt.xlabel('Glucose concentration [mg/dL]')
plt.grid()
plt.xticks(gridx)
plt.title("Linear regression on collected data")
plt.legend(["Data points", "Regressor"], loc="upper left")

mymodel3 = list(map(myfunc3, x3))
print("\nSlope3: ")
print(slope3)
print('Intercept3: ')
print(intercept3)

gridx = []
for i in range(80, 155, 5):
    gridx.append(i)

plt.figure(2)
plt.scatter(x3, y3, color='royalblue')
plt.plot(x3, mymodel3, color='red')
plt.ylabel('CA result [uA]')  # labeling the y label
plt.xlabel('Glucose concentration [mg/dL]')
plt.grid()
plt.xticks(gridx)
plt.title("Linear regression on collected data")
plt.legend(["Data points", "Regressor"], loc="upper left")
plt.show()
