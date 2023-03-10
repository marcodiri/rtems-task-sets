#!/usr/bin/env python3

import sys, getopt


def print_usage():
    print("Usage: convert_log.py -i <inputfile>")
    print("""Options:
          -i           <inputfile>
          -o           <outputfile>
          --timens     set this option if timestamps are nanoseconds.
          --no-sort    set this option to leave timestamps as they are.
          """)
    
def main(argv):
    inputfile = None
    outputfile = "relative_millisec.txt"
    timens = False
    no_sort = False
    
    opts, args = getopt.getopt(argv,"hi:o:",["timens","no-sort"])
    for opt, arg in opts:
        if opt == '-h':
            print_usage()
            sys.exit()
        elif opt in ("-i"):
            inputfile = arg
        elif opt in ("-o"):
            outputfile = arg
        elif opt in ("--timens"):
            timens = True
        elif opt in ("--no-sort"):
            no_sort = True
    
    if inputfile is None:
        print_usage()
        sys.exit()
    
    start, prev, prev_tmp = 0, 0, 0
    with open(inputfile, 'r') as file:
        log = file.read()
    spl = log.strip().split("\n")
    if not no_sort:
        # order timestamps (in case of preemption in the middle of logging)
        spl.sort(key=lambda el : int(el.split("-")[0]))
    with open(outputfile, "w") as f:
        for s in spl:
            ns, event = s.split("-")
            ns = int(ns)
            if event == "START":
                start = ns
            else:
                ns -= start
                prev_tmp = ns
                ns -= prev
                prev = prev_tmp
                if timens:
                    ms = round(float(ns)/1e6)  # if get_time is nanoseconds
                else:
                    ms = ns  # if get_time is milliseconds
                f.writelines((event, "\n", str(ms), "\n"))
        
    
    print(f"Output in {outputfile}")


if __name__ == "__main__":
    main(sys.argv[1:])
