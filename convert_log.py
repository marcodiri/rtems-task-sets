import sys, getopt


def print_usage():
    print("Usage: convert_log.py -i <inputfile>")
    print("""Options:
          -i <inputfile>
          -o <outputfile>
          --timens, set this option if timestamps are nanoseconds.
          """)

def main(argv):
    inputfile = None
    outputfile = "relative_millisec.txt"
    timens = False
    
    opts, args = getopt.getopt(argv,"hi:o:",["timens"])
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
    
    if inputfile is None:
        print_usage()
        sys.exit()
    
    start, prev, prev_tmp = 0, 0, 0
    with open(inputfile, 'r') as file:
        log = file.read()
    spl = log.strip().split("\n")
    with open(outputfile, "w") as f:
        for s in spl:
            ns, event = s.split("-")
            ns = int(ns)
            if event == "tsk1":
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
