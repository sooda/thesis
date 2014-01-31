import sys

def tree(filename):
    root = {"type": "folder", "ID": "(root)", "NAME": "(root)", "children": []}
    current_folder = root
    flat = [current_folder]
    stack = [] # does not include current
    current = current_folder

    for line in open(filename):
        line = line.rstrip()
        if line.startswith("#URL"):
            current = {"type": "url"}
            current_folder["children"].append(current)
            flat.append(current)
        elif line.startswith("#FOLDER"):
            stack.append(current_folder)
            current = {"type": "folder", "children": []}
            current_folder["children"].append(current)
            flat.append(current)
            current_folder = current
        elif line.startswith("\t"):
            optname, optval = line[1:].split("=", 1)
            current[optname] = optval
        elif line == "-":
            current_folder = stack.pop()

    return root, flat

def dumptree(foldernode, level=0):
    #import pprint
    #pprint.pprint(dippa)
    indent = "\t" * level

    print indent + foldernode["NAME"]

    for node in foldernode["children"]:
        if node["type"] == "url":
            print indent + "\t" + node["NAME"] + " " + node["URL"]

    for node in foldernode["children"]:
        if node["type"] == "folder":
            dumptree(node, level + 1)

def main():
    root, flat = tree(sys.argv[1])
    flatdict = dict((obj["NAME"], obj) for obj in flat)
    dippa = flatdict["dippa"]
    dumptree(dippa)

if __name__ == "__main__":
    main()
