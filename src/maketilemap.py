import random

mapsize = 1024 * 1024

f = open("./assets/tilemap.bin", "w+b")
b = [0 for i in range(0, mapsize)]
for i in range(0, mapsize):
    r = random.random()
    if r < 0.6:
        b[i] = 1
    elif r < 0.7:
        b[i] = 2
    elif r < 0.8:
        b[i] = 3
    elif r < 0.85:
        b[i] = 4
    elif r < 0.95:
        b[i] = 5
    else:
        b[i] = 6
bf = bytearray(b)
f.write(bf)
f.close()
