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

slope, intercept, r, p, std_err = stats.linregress(x, y)


def myfunc(x):
    return slope * x + intercept


mymodel = list(map(myfunc, x))
print("Slope: ")
print(slope)
print('\nIntercept: ')
print(intercept)

gridx = []
for i in range(80, 155, 5):
    gridx.append(i)

plt.scatter(x, y, color='royalblue')
plt.plot(x, mymodel, color='red')
plt.ylabel('CA result [uA]')  # labeling the y label
plt.xlabel('Glucose concentration [mg/dL]')
plt.grid()
plt.xticks(gridx)
plt.title("Linear regression on collected data")
plt.legend(["Data points", "Regressor"], loc="upper left")
plt.show()
