import os
import sys


if __name__ == "__main__":
    dir = sys.argv[1]
    output = open(dir + "Time.txt", "w")
    with open(dir + "nohup.out") as f:
        l = f.readlines()
        for line in l:
            if line.startswith("+=+="):
                output.write(line)
                output.write("\n")
    output.close()
    
                
