import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--integer", action="append")

args = parser.parse_args()

print(args.integer)
