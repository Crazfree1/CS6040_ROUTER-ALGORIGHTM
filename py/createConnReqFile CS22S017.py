import argparse
import random

def get_args():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-reqCnt',"--totalReqCount", type=int, default=300,
                        help="Number of connection requests program needs to create.")
    parser.add_argument('-mBW',"--maxBW", type=int, default=25,
                        help="Maximum Bandwidth that the connection request may ask for")
    parser.add_argument('-nC',"--nodeCount", type=int, default=20,
                        help="Total nuber of Nodes in the network")
    return parser.parse_args()

args = get_args()

connFile = 'SupplementoryFiles/connectionsfile.txt'
with open(connFile, 'w') as cFile:
    cFile.write(str(args.totalReqCount))
    cFile.write('\n')
    for i in range(int(args.totalReqCount)):
        cFile.write(str(random.randint(1, (args.nodeCount)-1)))
        cFile.write(' ')
        cFile.write(str(random.randint(1, (args.nodeCount)-1)))
        cFile.write(' ')
        BW = []
        BW.append(random.randint(1, args.maxBW))
        BW.append(random.randint(1, args.maxBW))
        BW.append(random.randint(1, args.maxBW))
        BW = sorted(BW)
        cFile.write(str(BW[0]))
        cFile.write(' ')
        cFile.write(str(BW[1]))
        cFile.write(' ')
        cFile.write(str(BW[2]))
        cFile.write('\n')

# Generate a random integer between 1 and 10 (inclusive)
#random_int = random.randint(1, 10)